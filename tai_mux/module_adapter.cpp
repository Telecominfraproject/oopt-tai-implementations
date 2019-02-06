#include "module_adapter.hpp"
#include <dlfcn.h>
#include <map>
#include <exception>
#include <sstream>
#include <iostream>

#define LOAD_TAI_API(name)                            \
    m_ ## name = (name ## _fn) dlsym(m_dl, #name);    \
    if ( m_ ## name == nullptr ) {                    \
        throw std::runtime_error( #name " undefined");\
    }

ModuleAdapter::ModuleAdapter(const std::string& name, uint64_t flags, const tai_service_method_table_t* services) : m_name(name) {
    m_dl = dlopen(name.c_str(), RTLD_NOW);
    if ( m_dl == nullptr ) {
        throw std::runtime_error(dlerror());
    }
    LOAD_TAI_API(tai_api_initialize)
    LOAD_TAI_API(tai_api_uninitialize)
    LOAD_TAI_API(tai_api_query)
    LOAD_TAI_API(tai_log_set)
    LOAD_TAI_API(tai_object_type_query)
    LOAD_TAI_API(tai_module_id_query)

    std::stringstream ss;

    auto status = tai_api_initialize(flags, services);
    if ( status != TAI_STATUS_SUCCESS ) {
        ss << "failed to initialize " << name << ":" << status;
        throw std::runtime_error(ss.str());
    }

    status = tai_api_query(TAI_API_MODULE, (void **)(&m_module_api));
    if ( status != TAI_STATUS_SUCCESS ) {
        ss << "failed to query TAI_API_MODULE :" << status;
        throw std::runtime_error(ss.str());
    }

    status = tai_api_query(TAI_API_NETWORKIF, (void **)(&m_netif_api));
    if ( status != TAI_STATUS_SUCCESS ) {
        ss << "failed to query TAI_API_NETWORKIF:" << status;
        throw std::runtime_error(ss.str());
    }

    status = tai_api_query(TAI_API_HOSTIF, (void **)(&m_hostif_api));
    if ( status != TAI_STATUS_SUCCESS ) {
        ss << "failed to query TAI_API_HOSTIF:" << status;
        throw std::runtime_error(ss.str());
    }
}

ModuleAdapter::~ModuleAdapter() {
    dlclose(m_dl);
}

uint64_t ModuleAdapter::dl_address(const std::string& name) {
    return (uint64_t)dlopen(name.c_str(), RTLD_NOW | RTLD_NOLOAD);
}
