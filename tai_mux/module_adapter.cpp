#include "module_adapter.hpp"
#include "exception.hpp"

#include <dlfcn.h>

#define LOAD_TAI_API(name)                            \
    m_ ## name = (name ## _fn) dlsym(m_dl, #name);    \
    if ( m_ ## name == nullptr ) {                    \
        throw std::runtime_error( #name " undefined");\
    }

namespace tai::mux {

    ModuleAdapter::ModuleAdapter(const std::string& name, uint64_t flags, const tai_service_method_table_t* services) : m_name(name) {
        m_dl = dlopen(name.c_str(), RTLD_NOW | RTLD_DEEPBIND);
        if ( m_dl == nullptr ) {
            throw std::runtime_error(dlerror());
        }
        LOAD_TAI_API(tai_api_initialize)
        LOAD_TAI_API(tai_api_uninitialize)
        LOAD_TAI_API(tai_api_query)
        LOAD_TAI_API(tai_log_set)
        LOAD_TAI_API(tai_object_type_query)
        LOAD_TAI_API(tai_module_id_query)

        auto status = tai_api_initialize(flags, services);
        if ( status != TAI_STATUS_SUCCESS ) {
            throw Exception(status);
        }

        status = tai_api_query(TAI_API_MODULE, (void **)(&m_module_api));
        if ( status != TAI_STATUS_SUCCESS ) {
            throw Exception(status);
        }

        status = tai_api_query(TAI_API_NETWORKIF, (void **)(&m_netif_api));
        if ( status != TAI_STATUS_SUCCESS ) {
            throw Exception(status);
        }

        status = tai_api_query(TAI_API_HOSTIF, (void **)(&m_hostif_api));
        if ( status != TAI_STATUS_SUCCESS ) {
            throw Exception(status);
        }
    }

    ModuleAdapter::~ModuleAdapter() {
        dlclose(m_dl);
    }

    uint64_t ModuleAdapter::dl_address(const std::string& name) {
        return (uint64_t)dlopen(name.c_str(), RTLD_NOW | RTLD_NOLOAD);
    }

}
