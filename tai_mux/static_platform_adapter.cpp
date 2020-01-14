#include "static_platform_adapter.hpp"

namespace tai::mux {

    StaticPlatformAdapter::StaticPlatformAdapter(uint64_t flags, const tai_service_method_table_t* services) {
        std::string config_file = TAI_MUX_STATIC_DEFAULT_CONFIG;
        auto e = std::getenv(TAI_MUX_STATIC_CONFIG_FILE.c_str());
        if ( e != nullptr ) {
            config_file = e;
        }

        std::ifstream ifs(config_file);
        if ( !ifs ) {
            return;
        }

        std::istreambuf_iterator<char> it(ifs), last;
        std::string config(it, last);

        std::map<uint64_t, S_ModuleAdapter> map;
        auto c = json::parse(config);

        m_services.module_presence = nullptr;
        if ( services != nullptr ) {
            m_services.get_module_io_handler = services->get_module_io_handler;
        }

        for ( json::iterator it = c.begin(); it != c.end(); ++it ) {
            auto location = it.key();
            auto lib = it.value().get<std::string>();
            auto dl = ModuleAdapter::dl_address(lib);
            S_ModuleAdapter ma;
            if ( dl == 0 ) {
                ma = std::make_shared<ModuleAdapter>(lib, flags, &m_services);
                dl = ModuleAdapter::dl_address(lib);
                map[dl] = ma;
            } else {
                ma = map[dl];
            }
            m_ma_map[location] = ma;
            if ( services != nullptr && services->module_presence != nullptr ) {
                services->module_presence(true, const_cast<char*>(location.c_str()));
            }
        }
    }

    StaticPlatformAdapter::~StaticPlatformAdapter() {
        for ( auto& m : m_ma_map ) {
            m.second->tai_api_uninitialize();
        }
    }
}
