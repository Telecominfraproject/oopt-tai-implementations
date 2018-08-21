#ifndef __STATIC_PLATFORM_ADAPTER_HPP__
#define __STATIC_PLATFORM_ADAPTER_HPP__

#include <thread>
#include <map>
#include <cstdlib>
#include <fstream>
#include <string>

#include "platform_adapter.hpp"
#include "module_adapter.hpp"
#include "tai.h"
#include "json.hpp"

using json = nlohmann::json;

const std::string TAI_MUX_STATIC_CONFIG_FILE = "TAI_MUX_STATIC_CONFIG_FILE";
const std::string TAI_MUX_STATIC_DEFAULT_CONFIG = "/etc/tai/mux/static.json";

class StaticPlatformAdapter : public PlatformAdapter {
    public:
        StaticPlatformAdapter(uint64_t flags, const tai_service_method_table_t* services);
        ~StaticPlatformAdapter();
        ModuleAdapter* get_module_adapter(const std::string& location) {
            return m_ma_map[location];
        };
    private:
        std::map<std::string, ModuleAdapter*> m_ma_map;
        std::thread m_th;
        tai_service_method_table_t m_services;
};

#endif
