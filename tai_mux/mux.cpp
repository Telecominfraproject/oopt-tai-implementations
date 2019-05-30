#include "mux.hpp"
#include <iostream>

Multiplexier *g_mux;

static const tai_attribute_value_t * find_attribute_in_list(
   _In_ tai_attr_id_t              attr_id,
   _In_ uint32_t                   attr_count,
   _In_ const tai_attribute_t     *attr_list)
{
    while (attr_count--) {
        if (attr_list->id == attr_id) {
            return &attr_list->value;
        }
        attr_list++;
    }
    return nullptr;
}

tai_status_t Multiplexier::create_host_interface(
        _Out_ tai_object_id_t *host_interface_id,
        _In_ tai_object_id_t module_id,
        _In_ uint32_t attr_count,
        _In_ const tai_attribute_t *attr_list) {
    tai_object_id_t m_id;
    ModuleAdapter *m_adapter;
    if ( g_mux == nullptr ) {
        return TAI_STATUS_UNINITIALIZED;
    }
    if ( g_mux->get_mapping(module_id, &m_adapter, &m_id) != 0 ) {
        return TAI_STATUS_FAILURE;
    }
    tai_object_id_t id;
    auto ret = m_adapter->create_host_interface(&id, m_id, attr_count, attr_list);
    if ( ret != TAI_STATUS_SUCCESS ) {
        return ret;
    }
    if ( g_mux->create_mapping(host_interface_id, m_adapter, id) != 0 ) {
        return TAI_STATUS_FAILURE;
    }
    return TAI_STATUS_SUCCESS;
}

tai_status_t Multiplexier::remove_host_interface(
        _In_ tai_object_id_t host_interface_id) {
    tai_object_id_t id;
    ModuleAdapter *m_adapter;
    if ( g_mux == nullptr ) {
        return TAI_STATUS_UNINITIALIZED;
    }

    if ( g_mux->get_mapping(host_interface_id, &m_adapter, &id) != 0 ) {
        return TAI_STATUS_FAILURE;
    }

    auto ret = m_adapter->remove_host_interface(id);
    if ( ret != TAI_STATUS_SUCCESS ) {
        return ret;
    }

    if ( g_mux->remove_mapping(host_interface_id) != 0 ) {
        return TAI_STATUS_FAILURE;
    }

    return TAI_STATUS_SUCCESS;
}

tai_status_t Multiplexier::set_host_interface_attributes(_In_ tai_object_id_t host_interface_id,
                                                 _In_ uint32_t attr_count,
                                                 _In_ const tai_attribute_t *attr_list) {
    tai_object_id_t id;
    ModuleAdapter *m_adapter;
    if ( g_mux == nullptr ) {
        return TAI_STATUS_UNINITIALIZED;
    }

    if ( g_mux->get_mapping(host_interface_id, &m_adapter, &id) != 0 ) {
        return TAI_STATUS_FAILURE;
    }
    return m_adapter->set_host_interface_attributes(id, attr_count, attr_list);
}

tai_status_t Multiplexier::get_host_interface_attributes(_In_ tai_object_id_t host_interface_id,
                                                 _In_ uint32_t attr_count,
                                                 _Out_ tai_attribute_t *attr_list) {
    tai_object_id_t id;
    ModuleAdapter *m_adapter;
    if ( g_mux == nullptr ) {
        return TAI_STATUS_UNINITIALIZED;
    }

    if ( g_mux->get_mapping(host_interface_id, &m_adapter, &id) != 0 ) {
        return TAI_STATUS_FAILURE;
    }
    return m_adapter->get_host_interface_attributes(id, attr_count, attr_list);
}

tai_status_t Multiplexier::create_network_interface(
        _Out_ tai_object_id_t *network_interface_id,
        _In_ tai_object_id_t module_id,
        _In_ uint32_t attr_count,
        _In_ const tai_attribute_t *attr_list) {
    tai_object_id_t m_id;
    ModuleAdapter *m_adapter;
    if ( g_mux == nullptr ) {
        return TAI_STATUS_UNINITIALIZED;
    }
    if ( g_mux->get_mapping(module_id, &m_adapter, &m_id) != 0 ) {
        return TAI_STATUS_FAILURE;
    }
    tai_object_id_t id;
    auto ret = m_adapter->create_network_interface(&id, m_id, attr_count, attr_list);
    if ( ret != TAI_STATUS_SUCCESS ) {
        return ret;
    }
    if ( g_mux->create_mapping(network_interface_id, m_adapter, id) != 0 ) {
        return TAI_STATUS_FAILURE;
    }
    return TAI_STATUS_SUCCESS;
}

tai_status_t Multiplexier::remove_network_interface(
        _In_ tai_object_id_t network_interface_id) {
    tai_object_id_t id;
    ModuleAdapter *m_adapter;
    if ( g_mux == nullptr ) {
        return TAI_STATUS_UNINITIALIZED;
    }

    if ( g_mux->get_mapping(network_interface_id, &m_adapter, &id) != 0 ) {
        return TAI_STATUS_FAILURE;
    }

    auto ret = m_adapter->remove_network_interface(id);
    if ( ret != TAI_STATUS_SUCCESS ) {
        return ret;
    }

    if ( g_mux->remove_mapping(network_interface_id) != 0 ) {
        return TAI_STATUS_FAILURE;
    }

    return TAI_STATUS_SUCCESS;
}

tai_status_t Multiplexier::set_network_interface_attributes(_In_ tai_object_id_t network_interface_id,
                                                 _In_ uint32_t attr_count,
                                                 _In_ const tai_attribute_t *attr_list) {
    tai_object_id_t id;
    ModuleAdapter *m_adapter;
    if ( g_mux == nullptr ) {
        return TAI_STATUS_UNINITIALIZED;
    }

    if ( g_mux->get_mapping(network_interface_id, &m_adapter, &id) != 0 ) {
        return TAI_STATUS_FAILURE;
    }
    return m_adapter->set_network_interface_attributes(id, attr_count, attr_list);
}

tai_status_t Multiplexier::get_network_interface_attributes(_In_ tai_object_id_t network_interface_id,
                                                 _In_ uint32_t attr_count,
                                                 _Out_ tai_attribute_t *attr_list) {
    tai_object_id_t id;
    ModuleAdapter *m_adapter;
    if ( g_mux == nullptr ) {
        return TAI_STATUS_UNINITIALIZED;
    }

    if ( g_mux->get_mapping(network_interface_id, &m_adapter, &id) != 0 ) {
        return TAI_STATUS_FAILURE;
    }
    return m_adapter->get_network_interface_attributes(id, attr_count, attr_list);
}

tai_status_t Multiplexier::create_module(
    _Out_ tai_object_id_t          *module_id,
    _In_ uint32_t                   attr_count,
    _In_ const tai_attribute_t     *attr_list) {
    if ( g_mux == nullptr ) {
        return TAI_STATUS_UNINITIALIZED;
    }
    auto mod_addr = find_attribute_in_list(TAI_MODULE_ATTR_LOCATION, attr_count, attr_list);
    if ( mod_addr == nullptr ) {
        return TAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    const std::string location(mod_addr->charlist.list);

    auto m_adapter = g_mux->get_module_adapter(location);

    if ( m_adapter == nullptr ) {
        return TAI_STATUS_FAILURE;
    }

    tai_object_id_t id;

    auto ret = m_adapter->create_module(&id, attr_count, attr_list);
    if ( ret != TAI_STATUS_SUCCESS ) {
        return ret;
    }

    if ( g_mux->create_mapping(module_id, m_adapter, id) != 0 ) {
        return TAI_STATUS_FAILURE;
    }

    return TAI_STATUS_SUCCESS;
}

tai_status_t Multiplexier::remove_module(_In_ tai_object_id_t module_id) {
    tai_object_id_t id;
    ModuleAdapter *m_adapter;
    if ( g_mux == nullptr ) {
        return TAI_STATUS_UNINITIALIZED;
    }

    if ( g_mux->get_mapping(module_id, &m_adapter, &id) != 0 ) {
        return TAI_STATUS_FAILURE;
    }

    auto ret = m_adapter->remove_module(id);
    if ( ret != TAI_STATUS_SUCCESS ) {
        return ret;
    }

    if ( g_mux->remove_mapping(module_id) != 0 ) {
        return TAI_STATUS_FAILURE;
    }

    return TAI_STATUS_SUCCESS;
}

tai_status_t Multiplexier::set_module_attributes(_In_ tai_object_id_t module_id,
                                                 _In_ uint32_t attr_count,
                                                 _In_ const tai_attribute_t *attr_list) {
    tai_object_id_t id;
    ModuleAdapter *m_adapter;
    if ( g_mux == nullptr ) {
        return TAI_STATUS_UNINITIALIZED;
    }

    if ( g_mux->get_mapping(module_id, &m_adapter, &id) != 0 ) {
        return TAI_STATUS_FAILURE;
    }
    return m_adapter->set_module_attributes(id, attr_count, attr_list);
}

tai_status_t Multiplexier::get_module_attributes(_In_ tai_object_id_t module_id,
                                                 _In_ uint32_t attr_count,
                                                 _Out_ tai_attribute_t *attr_list) {
    tai_object_id_t id;
    ModuleAdapter *m_adapter;
    if ( g_mux == nullptr ) {
        return TAI_STATUS_UNINITIALIZED;
    }

    if ( g_mux->get_mapping(module_id, &m_adapter, &id) != 0 ) {
        return TAI_STATUS_FAILURE;
    }
    return m_adapter->get_module_attributes(id, attr_count, attr_list);
}

tai_object_type_t Multiplexier::object_type_query(_In_ tai_object_id_t id) {
    tai_object_id_t realid;
    ModuleAdapter *m_adapter;
    if ( g_mux == nullptr ) {
        return TAI_OBJECT_TYPE_NULL;
    }

    if ( g_mux->get_mapping(id, &m_adapter, &realid) != 0 ) {
        return TAI_OBJECT_TYPE_NULL;
    }
    return m_adapter->tai_object_type_query(realid);
}

tai_status_t Multiplexier::tai_log_set(_In_ tai_api_t api, _In_ tai_log_level_t level) {
    if ( g_mux == nullptr ) {
        return TAI_STATUS_UNINITIALIZED;
    }
    auto set = m_pa->list_module_adapters();
    for ( const auto& a :  set ) {
        auto ret = a->tai_log_set(api, level);
        if ( ret != TAI_STATUS_SUCCESS ) {
            return ret;
        }
    }
    return TAI_STATUS_SUCCESS;
}

Multiplexier* create_mux(platform_adapter_t pa_kind, uint64_t flags, const tai_service_method_table_t* services) {
    try {
        return new Multiplexier(pa_kind, flags, services);
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return nullptr;
    }
}
