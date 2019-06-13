#include "mux.hpp"
#include <cstdlib>
#include <iostream>

extern Multiplexier *g_mux;

const std::string PLATFORM_ADAPTER = "TAI_MUX_PLATFORM_ADAPTER";

#ifndef DEFAULT_PLATFORM_ADAPTER
#define DEFAULT_PLATFORM_ADAPTER "static"
#endif

tai_status_t tai_api_initialize(_In_ uint64_t flags,
                                _In_ const tai_service_method_table_t* services)
{
    if ( g_mux != nullptr ) {
        return TAI_STATUS_SUCCESS;
    }
    auto pa = std::getenv(PLATFORM_ADAPTER.c_str());
    std::string pa_name;
    if ( pa == nullptr ) {
        pa_name = DEFAULT_PLATFORM_ADAPTER;
    } else {
        pa_name = std::string(pa);
    }
    platform_adapter_t pa_kind;
    if ( pa_name == "static" ) {
        pa_kind = PLATFORM_ADAPTER_STATIC;
    }
    g_mux = create_mux(pa_kind, flags, services);
    if ( g_mux == nullptr ) {
        return TAI_STATUS_FAILURE;
    }
    return TAI_STATUS_SUCCESS;
}

tai_status_t tai_api_uninitialize(void) {
    if ( g_mux != nullptr ) {
        delete g_mux;
        g_mux = nullptr;
    }
    return TAI_STATUS_SUCCESS;
}

static tai_status_t mux_create_host_interface(
    _Out_ tai_object_id_t *host_interface_id,
    _In_ tai_object_id_t module_id,
    _In_ uint32_t attr_count,
    _In_ const tai_attribute_t *attr_list) {
    return g_mux->create_host_interface(host_interface_id, module_id, attr_count, attr_list);
}

static tai_status_t mux_remove_host_interface(_In_ tai_object_id_t host_interface_id)
{
    return g_mux->remove_host_interface(host_interface_id);
}

static tai_status_t mux_set_host_interface_attributes(
   _In_ tai_object_id_t        host_interface_id,
   _In_ uint32_t               attr_count,
   _In_ const tai_attribute_t *attr_list)
{
    return g_mux->set_host_interface_attributes(host_interface_id, attr_count, attr_list);
}

static tai_status_t mux_set_host_interface_attribute(
   _In_ tai_object_id_t        host_interface_id,
   _In_ const tai_attribute_t *attr)
{
    return g_mux->set_host_interface_attributes(host_interface_id, 1, attr);
}

static tai_status_t mux_get_host_interface_attributes(
   _In_ tai_object_id_t        host_interface_id,
   _In_ uint32_t               attr_count,
   _Out_ tai_attribute_t *attr_list)
{
    return g_mux->get_host_interface_attributes(host_interface_id, attr_count, attr_list);
}

static tai_status_t mux_get_host_interface_attribute(
   _In_ tai_object_id_t        host_interface_id,
   _Out_ tai_attribute_t *attr)
{
    return g_mux->get_host_interface_attributes(host_interface_id, 1, attr);
}

tai_host_interface_api_t mux_host_interface_api = {
    .create_host_interface         = mux_create_host_interface,
    .remove_host_interface         = mux_remove_host_interface,
    .set_host_interface_attribute  = mux_set_host_interface_attribute,
    .set_host_interface_attributes = mux_set_host_interface_attributes,
    .get_host_interface_attribute  = mux_get_host_interface_attribute,
    .get_host_interface_attributes = mux_get_host_interface_attributes
};

static tai_status_t mux_create_network_interface(
    _Out_ tai_object_id_t *network_interface_id,
    _In_ tai_object_id_t module_id,
    _In_ uint32_t attr_count,
    _In_ const tai_attribute_t *attr_list) {
    return g_mux->create_network_interface(network_interface_id, module_id, attr_count, attr_list);
}

static tai_status_t mux_remove_network_interface(_In_ tai_object_id_t network_interface_id)
{
    return g_mux->remove_network_interface(network_interface_id);
}

static tai_status_t mux_set_network_interface_attributes(
   _In_ tai_object_id_t        network_interface_id,
   _In_ uint32_t               attr_count,
   _In_ const tai_attribute_t *attr_list)
{
    return g_mux->set_network_interface_attributes(network_interface_id, attr_count, attr_list);
}

static tai_status_t mux_set_network_interface_attribute(
   _In_ tai_object_id_t        network_interface_id,
   _In_ const tai_attribute_t *attr)
{
    return g_mux->set_network_interface_attributes(network_interface_id, 1, attr);
}

static tai_status_t mux_get_network_interface_attributes(
   _In_ tai_object_id_t        network_interface_id,
   _In_ uint32_t               attr_count,
   _Out_ tai_attribute_t *attr_list)
{
    return g_mux->get_network_interface_attributes(network_interface_id, attr_count, attr_list);
}

static tai_status_t mux_get_network_interface_attribute(
   _In_ tai_object_id_t        network_interface_id,
   _Out_ tai_attribute_t *attr)
{
    return g_mux->get_network_interface_attributes(network_interface_id, 1, attr);
}

tai_network_interface_api_t mux_network_interface_api = {
    .create_network_interface         = mux_create_network_interface,
    .remove_network_interface         = mux_remove_network_interface,
    .set_network_interface_attribute  = mux_set_network_interface_attribute,
    .set_network_interface_attributes = mux_set_network_interface_attributes,
    .get_network_interface_attribute  = mux_get_network_interface_attribute,
    .get_network_interface_attributes = mux_get_network_interface_attributes
};

static tai_status_t mux_create_module(
    _Out_ tai_object_id_t          *module_id,
    _In_ uint32_t                   attr_count,
    _In_ const tai_attribute_t     *attr_list)
{
    return g_mux->create_module(module_id, attr_count, attr_list);
}

static tai_status_t mux_remove_module(_In_ tai_object_id_t module_id)
{
    return g_mux->remove_module(module_id);
}

static tai_status_t mux_set_module_attributes(
   _In_ tai_object_id_t        module_id,
   _In_ uint32_t               attr_count,
   _In_ const tai_attribute_t *attr_list)
{
    return g_mux->set_module_attributes(module_id, attr_count, attr_list);
}

static tai_status_t mux_set_module_attribute(
    _In_ tai_object_id_t    module_id,
    _In_ const tai_attribute_t *attr)
{
    return mux_set_module_attributes(module_id, 1, attr);
}

static tai_status_t mux_get_module_attributes(
   _In_ tai_object_id_t        module_id,
   _In_ uint32_t               attr_count,
   _Out_ tai_attribute_t *attr_list)
{
    return g_mux->get_module_attributes(module_id, attr_count, attr_list);
}

static tai_status_t mux_get_module_attribute(
   _In_ tai_object_id_t        module_id,
   _Out_ tai_attribute_t *attr)
{
    return mux_get_module_attributes(module_id, 1, attr);
}

tai_module_api_t mux_module_api = {
    .create_module         = mux_create_module,
    .remove_module         = mux_remove_module,
    .set_module_attribute = mux_set_module_attribute,
    .set_module_attributes = mux_set_module_attributes,
    .get_module_attribute  = mux_get_module_attribute,
    .get_module_attributes = mux_get_module_attributes
};

tai_status_t tai_api_query(_In_ tai_api_t tai_api_id,
                           _Out_ void** api_method_table)
{
    if ( api_method_table == nullptr ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    switch (tai_api_id) {
    case TAI_API_MODULE:
        *api_method_table = &mux_module_api;
        break;
    case TAI_API_HOSTIF:
        *api_method_table = &mux_host_interface_api;
        break;
    case TAI_API_NETWORKIF:
        *api_method_table = &mux_network_interface_api;
        break;
    default:
        return TAI_STATUS_INVALID_PARAMETER;
    }
    return TAI_STATUS_SUCCESS;
}

tai_status_t tai_log_set(tai_api_t tai_api_id, tai_log_level_t log_level) {
    return g_mux->tai_log_set(tai_api_id, log_level);
}

tai_object_type_t tai_object_type_query(tai_object_id_t id) {
    return g_mux->object_type_query(id);
}

tai_object_id_t tai_module_id_query(tai_object_id_t id) {
    return g_mux->module_id_query(id);
}
