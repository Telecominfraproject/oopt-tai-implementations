#include "mux.hpp"
#include <iostream>
#include "taimetadata.h"

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
    return set_attributes(&ModuleAdapter::set_host_interface_attributes, host_interface_id, attr_count, attr_list);
}

tai_status_t Multiplexier::get_host_interface_attributes(_In_ tai_object_id_t host_interface_id,
                                                 _In_ uint32_t attr_count,
                                                 _Out_ tai_attribute_t *attr_list) {
    return get_attributes(&ModuleAdapter::get_host_interface_attributes, host_interface_id, attr_count, attr_list);
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
    return set_attributes(&ModuleAdapter::set_network_interface_attributes, network_interface_id, attr_count, attr_list);
}

tai_status_t Multiplexier::get_network_interface_attributes(_In_ tai_object_id_t network_interface_id,
                                                 _In_ uint32_t attr_count,
                                                 _Out_ tai_attribute_t *attr_list) {
    return get_attributes(&ModuleAdapter::get_network_interface_attributes, network_interface_id, attr_count, attr_list);
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
    return set_attributes(&ModuleAdapter::set_module_attributes, module_id, attr_count, attr_list);
}

tai_status_t Multiplexier::get_module_attributes(_In_ tai_object_id_t module_id,
                                                 _In_ uint32_t attr_count,
                                                 _Out_ tai_attribute_t *attr_list) {
    return get_attributes(&ModuleAdapter::get_module_attributes, module_id, attr_count, attr_list);
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

void notification_callback(void* context, tai_object_id_t oid, tai_attribute_t const * const attribute) {
    if ( context == nullptr || attribute == nullptr ) {
        return;
    }
    auto ctx = static_cast<notification_context*>(context);
    ctx->mux->notify(ctx, oid, attribute);
}

void Multiplexier::notify(notification_context* ctx, tai_object_id_t real_oid, tai_attribute_t const * const src) {
    auto oid = get_reverse_mapping(real_oid, ctx->adapter);
    if ( oid == TAI_NULL_OBJECT_ID ) {
        return;
    }
    auto key = std::pair<tai_object_id_t, tai_attr_id_t>(oid, ctx->notify_id);
    tai_attribute_t dst;
    tai_alloc_info_t info;
    info.reference = src;
    auto t = object_type_query(oid);

    auto meta = tai_metadata_get_attr_metadata(t, src->id);
    if ( meta == nullptr ) {
        return;
    }

    dst.id = src->id;

    if ( tai_metadata_alloc_attr_value(meta, &dst, &info) != 0 ) {
        return;
    }

    if ( tai_metadata_deepcopy_attr_value(meta, src, &dst) != 0 ) {
        goto err;
    }

    if ( convert_oid(oid, &dst, &dst, true) != TAI_STATUS_SUCCESS ) {
        goto err;
    }

    ctx->handler.notify(ctx->handler.context, oid, &dst);
err:
    tai_metadata_free_attr_value(meta, &dst, nullptr);
}

tai_status_t Multiplexier::set_attributes( std::function<tai_status_t(ModuleAdapter*, tai_object_id_t, uint32_t, const tai_attribute_t*)> f, tai_object_id_t oid, uint32_t attr_count, const tai_attribute_t *attr_list) {
    tai_object_id_t id;
    ModuleAdapter *m_adapter;
    tai_status_t ret;
    if ( g_mux == nullptr ) {
        return TAI_STATUS_UNINITIALIZED;
    }

    if ( g_mux->get_mapping(oid, &m_adapter, &id) != 0 ) {
        return TAI_STATUS_FAILURE;
    }

    std::vector<tai_attribute_t> attrs;
    tai_alloc_info_t info;
    auto t = object_type_query(oid);
    for ( auto i = 0; i < attr_count; i++ ) {
        auto meta = tai_metadata_get_attr_metadata(t, attr_list[i].id);
        if ( meta == nullptr ) {
            goto err;
        }

        tai_attribute_t attr;
        attr.id = attr_list[i].id;
        info.reference = &attr_list[i];
        if ( tai_metadata_alloc_attr_value(meta, &attr, &info) != 0 ) {
            goto err;
        }
        if ( tai_metadata_deepcopy_attr_value(meta, &attr_list[i], &attr) != 0 ) {
            goto err;
        }
        ret = convert_oid(oid, &attr, &attr, false);
        if ( ret != TAI_STATUS_SUCCESS ) {
            goto err;
        }
        attrs.emplace_back(attr);
    }
    ret = f(m_adapter, id, attrs.size(), attrs.data());
err:
    if ( free_attributes(t, attrs) < 0 ) {
        return TAI_STATUS_FAILURE;
    }
    return ret;
}

tai_status_t Multiplexier::get_attributes( std::function<tai_status_t(ModuleAdapter*, tai_object_id_t, uint32_t, tai_attribute_t*)> f, tai_object_id_t oid, uint32_t attr_count, tai_attribute_t *attr_list) {
    tai_object_id_t id;
    ModuleAdapter *m_adapter;
    if ( g_mux == nullptr ) {
        return TAI_STATUS_UNINITIALIZED;
    }

    if ( g_mux->get_mapping(oid, &m_adapter, &id) != 0 ) {
        return TAI_STATUS_FAILURE;
    }

    auto ret = f(m_adapter, id, attr_count, attr_list);
    if ( ret != TAI_STATUS_SUCCESS ) {
        return ret;
    }

    auto t = object_type_query(oid);
    for ( auto i = 0; i < attr_count; i++ ) {
        ret = convert_oid(oid, &attr_list[i], &attr_list[i], true);
        if ( ret != TAI_STATUS_SUCCESS ) {
            return ret;
        }
    }
    return TAI_STATUS_SUCCESS;
}

tai_status_t Multiplexier::convert_oid(tai_object_id_t oid, const tai_attribute_t *src, tai_attribute_t *dst, bool reversed) {
    ModuleAdapter* adapter;
    auto t = object_type_query(oid);
    if ( g_mux->get_mapping(oid, &adapter, nullptr) != 0 ) {
        return TAI_STATUS_FAILURE;
    }
    auto meta = tai_metadata_get_attr_metadata(t, src->id);
    const tai_object_map_list_t *oml;
    auto convert = [&](tai_object_id_t s) -> tai_object_id_t {
        if ( reversed ) {
            return g_mux->get_reverse_mapping(s, adapter);
        }
        tai_object_id_t oid;
        if ( g_mux->get_mapping(s, nullptr, &oid) < 0 ) {
            return TAI_NULL_OBJECT_ID;
        }
        return oid;
    };
    auto key = std::pair<tai_object_id_t, tai_attr_id_t>(oid, src->id);

    switch (meta->attrvaluetype) {
    case TAI_ATTR_VALUE_TYPE_OID:
        dst->value.oid = convert(src->value.oid);
        if ( dst->value.oid == TAI_NULL_OBJECT_ID ) {
            return TAI_STATUS_FAILURE;
        }
        break;
    case TAI_ATTR_VALUE_TYPE_OBJLIST:
        for ( auto i = 0 ; i < src->value.objlist.count; i++ ) {
            dst->value.objlist.list[i] = convert(src->value.objlist.list[i]);
            if ( dst->value.objlist.list[i] == TAI_NULL_OBJECT_ID ) {
                return TAI_STATUS_FAILURE;
            }
        }
        break;
    case TAI_ATTR_VALUE_TYPE_OBJMAPLIST:
        oml = &src->value.objmaplist;
        for ( auto i = 0 ; i < oml->count; i++ ) {
            dst->value.objmaplist.list[i].key = convert(oml->list[i].key);
            if ( dst->value.objmaplist.list[i].key == TAI_NULL_OBJECT_ID ) {
                return TAI_STATUS_FAILURE;
            }
            for ( auto j = 0; j < oml->list[i].value.count; j++ ) {
                dst->value.objmaplist.list[i].value.list[j] = convert(oml->list[i].value.list[j]);
                if ( dst->value.objmaplist.list[i].value.list[j] == TAI_NULL_OBJECT_ID ) {
                    return TAI_STATUS_FAILURE;
                }
            }
        }
        break;
    case TAI_ATTR_VALUE_TYPE_NOTIFICATION:
        if ( reversed ) {
            if ( m_notification_map.find(key) != m_notification_map.end() ) {
                auto n = m_notification_map[key];
                dst->value.notification.context = n->handler.context;
                dst->value.notification.notify = n->handler.notify;
            }
        } else {
            if ( src->value.notification.notify == nullptr ) {
                delete m_notification_map[key];
                m_notification_map.erase(key);
            } else {
                if ( m_notification_map.find(key) == m_notification_map.end() ) {
                    m_notification_map[key] = new notification_context();
                }
                auto n = m_notification_map[key];
                n->mux = this;
                n->adapter = adapter;
                n->handler = src->value.notification;
                n->notify_id = src->id;
                dst->value.notification.context = n;
                dst->value.notification.notify = notification_callback;
            }
        }
    }
    return TAI_STATUS_SUCCESS;
}

int Multiplexier::free_attributes(tai_object_type_t t, std::vector<tai_attribute_t>& attributes) {
    for ( auto a : attributes ) {
        auto meta = tai_metadata_get_attr_metadata(t, a.id);
        tai_metadata_free_attr_value(meta, &a, nullptr);
    }
    return 0;
}

Multiplexier* create_mux(platform_adapter_t pa_kind, uint64_t flags, const tai_service_method_table_t* services) {
    try {
        return new Multiplexier(pa_kind, flags, services);
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return nullptr;
    }
}
