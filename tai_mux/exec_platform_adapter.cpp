#include "exec_platform_adapter.hpp"
#include <cstdlib>
#include <cstdio>
#include <sstream>

namespace tai::mux {

    // trim from end (in place)
    static inline void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
            return std::isgraph(ch);
        }).base(), s.end());
    }

    static int exec(const std::string& cmd, std::string& output) {
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) return -1;
        char buffer[128] = {};
        while (!feof(pipe)) {
            if (fgets(buffer, 128, pipe) != NULL)
                output += buffer;
        }
        return WEXITSTATUS(pclose(pipe));
    }

    static int exec_script(const std::string& arg, std::string& output) {
        std::stringstream ss;
        std::string script = TAI_MUX_EXEC_DEFAULT_SCRIPT;
        auto e = std::getenv(TAI_MUX_EXEC_SCRIPT.c_str());
        if ( e != nullptr ) {
            script = e;
        }
        ss << script;
        ss << " ";
        ss << arg;
        auto ret = exec(ss.str(), output);
        if ( ret != 0 ) {
            TAI_ERROR("failed to execute %s: ret: %d", script.c_str(), ret);
            return ret;
        }
        rtrim(output);
        return 0;
    }

    ExecPlatformAdapter::ExecPlatformAdapter(uint64_t flags, const tai_service_method_table_t* services) : m_flags(flags) {

        m_services.module_presence = nullptr;
        if ( services != nullptr ) {
            m_services.get_module_io_handler = services->get_module_io_handler;
        }

        if ( services != nullptr && services->module_presence != nullptr ) {
            std::string output;
            auto ret = exec_script("list", output);
            if ( ret != 0 ) {
                throw Exception(TAI_STATUS_FAILURE);
            }
            TAI_DEBUG("result of list: %s", output.c_str());
            std::stringstream ss(output);
            std::string location;
            while(getline(ss, location)) {
                services->module_presence(true, const_cast<char*>(location.c_str()));
            }
        }
   }

    ExecPlatformAdapter::~ExecPlatformAdapter() {
        for ( auto& m : m_ma_map ) {
            m.second->tai_api_uninitialize();
        }
    }

    S_ModuleAdapter ExecPlatformAdapter::get_module_adapter(const std::string& location) {
        std::string lib;
        auto ret = exec_script(location, lib);
        if ( ret != 0 ) {
            TAI_ERROR("script failed: %d", ret);
            return nullptr;
        }
        if ( lib == "" ) {
            TAI_ERROR("no library found");
            return nullptr;
        }
        auto dl = ModuleAdapter::dl_address(lib);
        S_ModuleAdapter ma;
        TAI_DEBUG("dl: %p, lib: %s", dl, lib.c_str());
        if ( dl == 0 ) {
            ma = std::make_shared<ModuleAdapter>(lib, m_flags, &m_services);
            dl = ModuleAdapter::dl_address(lib);
            m_lib_map[dl] = ma;
        } else {
            ma = m_lib_map[dl];
        }
        m_ma_map[location] = ma;
        return ma;
    }
}
