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

    S_ModuleAdapter StaticPlatformAdapter::get_module_adapter(const std::string& location) {

	std::string config_file = TAI_MUX_STATIC_DEFAULT_CONFIG;
        auto e = std::getenv(TAI_MUX_STATIC_CONFIG_FILE.c_str());
        if ( e != nullptr ) {
            config_file = e;
        }

        std::ifstream ifs(config_file);

        std::istreambuf_iterator<char> it(ifs), last;
        std::string config(it, last);

        std::map<uint64_t, S_ModuleAdapter> map;
        auto c = json::parse(config);

	 for ( json::iterator it = c.begin(); it != c.end(); ++it ) {
            auto slot = it.key();
	    if (slot == location) {
		    auto new_name = it.value().get<std::string>();
		    auto dl_get = ModuleAdapter::dl_address(new_name);

		    if (dl_get == 0) {
			    S_ModuleAdapter ma_get;
			    ma_get = std::make_shared<ModuleAdapter>(new_name, 0, &m_services);
				m_ma_map[location] = ma_get;
			}
		    else if (!m_ma_map[location]->match_m_dl(dl_get)) {
        		for (auto& index : m_ma_map) {
					if (index.first == location)
						continue;
					else if(index.second->match_m_dl(dl_get))
					{
						m_ma_map[location] = index.second;
						break;
					}
				}
			}
		    break;
	     }
	 }  
	 return m_ma_map[location];
    }
    	


    StaticPlatformAdapter::~StaticPlatformAdapter() {
        for ( auto& m : m_ma_map ) {
            m.second->tai_api_uninitialize();
        }
    }
}
