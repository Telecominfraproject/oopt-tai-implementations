#include "platform_adapter.hpp"
#include "module_adapter.hpp"
#include <algorithm>
#include <cstring>

namespace tai::mux {

    static void notification_callback(void* context, tai_object_id_t oid, uint32_t attr_count, tai_attribute_t const * const attr_list) {
        if ( context == nullptr || attr_list == nullptr ) {
            return;
        }
        auto ctx = static_cast<NotificationContext*>(context);
        if ( ctx->pa != nullptr ) {
            ctx->pa->notify(ctx, oid, attr_count, attr_list);
        }
    }

    void PlatformAdapter::notify(NotificationContext* ctx, tai_object_id_t real_oid, uint32_t attr_count, tai_attribute_t const * const attr_list) {
        auto oid = ctx->muxed_oid;
        std::vector<S_Attribute> attrs;
        for ( int i = 0; i < attr_count; i++ ) {
            auto src = attr_list[i];
            auto meta = tai_metadata_get_attr_metadata(ctx->object_type, src.id);
            if ( meta == nullptr ) {
                continue;
            }
            auto dst = std::make_shared<Attribute>(meta, src);
            auto ret = convert_oid(ctx->object_type, oid, dst, dst, true);
            if ( ret != TAI_STATUS_SUCCESS ) {
                ERROR("failed to convert oid of attribute: %d", src.id);
                continue;
            }
            attrs.emplace_back(dst);
        }
        std::vector<tai_attribute_t> raw_attrs;
        std::transform(attrs.begin(), attrs.end(), std::back_inserter(raw_attrs), [](S_Attribute a) { return *a->raw(); });
        std::unique_lock<std::mutex> lk(ctx->mutex);
        ctx->real_handler.notify(ctx->real_handler.context, oid, raw_attrs.size(), raw_attrs.data());
    }

    tai_status_t PlatformAdapter::convert_oid(const tai_object_type_t& type, const tai_object_id_t& id, const S_ConstAttribute src, const S_Attribute dst, bool reversed) {
        return convert_oid(type, id, src->raw(), const_cast<tai_attribute_t* const>(dst->raw()), reversed);
    }

    tai_status_t PlatformAdapter::convert_oid(const tai_object_type_t& type, const tai_object_id_t& id, const tai_attribute_t * const src, tai_attribute_t * const dst, bool reversed) {
        auto meta = tai_metadata_get_attr_metadata(type, src->id);
        const tai_object_map_list_t *oml;
        S_ModuleAdapter adapter;
        if ( get_mapping(id, &adapter, nullptr) != 0 ) {
            return TAI_STATUS_FAILURE;
        }
        auto convert = [&](tai_object_id_t s) -> tai_object_id_t {
            if ( reversed ) {
                return get_reverse_mapping(s, adapter);
            }
            tai_object_id_t oid;
            if ( get_mapping(s, nullptr, &oid) < 0 ) {
                return TAI_NULL_OBJECT_ID;
            }
            return oid;
        };

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
            {
                auto key = notification_key(id, src->id);
                if ( reversed ) {
                    if ( m_notification_map.find(key) != m_notification_map.end() ) {
                        auto n = m_notification_map[key];
                        dst->value.notification.context = n->real_handler.context;
                        dst->value.notification.notify = n->real_handler.notify;
                    }
                    break;
                }

                if ( src->value.notification.notify != nullptr ) {
                    if ( m_notification_map.find(key) == m_notification_map.end() ) {
                        m_notification_map[key] = std::make_shared<NotificationContext>();
                    }
                    auto n = m_notification_map[key];
                    std::unique_lock<std::mutex> lk(n->mutex);
                    n->pa = this;
                    n->real_handler = src->value.notification;
                    n->muxed_oid = id;
                    n->object_type = type;
                    dst->value.notification.context = static_cast<void*>(n.get());
                    dst->value.notification.notify = notification_callback;
                } else {
                    // notification context cleanup can't be handled here since the callback can still get
                    // called by the underneath TAI library.
                    // we'll clean the context after disabling the callback
                }
            }
        }
        return TAI_STATUS_SUCCESS;
    }

    tai_status_t PlatformAdapter::get(const tai_object_type_t& type, const tai_object_id_t& id, uint32_t count, tai_attribute_t* const attrs) {
        S_ModuleAdapter adapter;
        tai_object_id_t real_id;
        if ( get_mapping(id, &adapter, &real_id) != 0 ) {
            return TAI_STATUS_FAILURE;
        }
        auto ret = adapter->get_attributes(type, real_id, count, attrs);
        if ( ret != TAI_STATUS_SUCCESS ) {
            return ret;
        }
        for ( auto i = 0; i < count; i++ ) {
            auto attribute = &attrs[i];
            auto ret = convert_oid(type, id, attribute, attribute, true);
            if ( ret != TAI_STATUS_SUCCESS ) {
                return ret;
            }
        }
        return TAI_STATUS_SUCCESS;
    }

    tai_status_t PlatformAdapter::set(const tai_object_type_t& type, const tai_object_id_t& id, uint32_t count, const tai_attribute_t* const attrs) {
        S_ModuleAdapter adapter;
        tai_object_id_t real_id;
        if ( get_mapping(id, &adapter, &real_id) != 0 ) {
            return TAI_STATUS_FAILURE;
        }
        std::vector<S_Attribute> ptrs;
        std::vector<tai_attribute_t> inputs;
        std::vector<notification_key> keys_to_remove;

        for ( auto i = 0; i < count; i++ ) {
            auto attribute = &attrs[i];
            auto meta = tai_metadata_get_attr_metadata(type, attribute->id);
            if ( meta == nullptr ) {
                return TAI_STATUS_FAILURE;
            }
            auto attr = std::make_shared<Attribute>(meta, attribute);
            ptrs.emplace_back(attr); // just for memory management
            auto ret = convert_oid(type, id, attribute, const_cast<tai_attribute_t* const>(attr->raw()), false);
            if ( ret != TAI_STATUS_SUCCESS ) {
                return ret;
            }
            inputs.emplace_back(*attr->raw());
            if ( meta->attrvaluetype == TAI_ATTR_VALUE_TYPE_NOTIFICATION && attribute->value.notification.notify == nullptr ) {
                keys_to_remove.emplace_back(notification_key(id, attribute->id));
            }
        }

        auto ret = adapter->set_attributes(type, real_id, inputs.size(), inputs.data());
        if ( ret != TAI_STATUS_SUCCESS ) {
            return ret;
        }

        for ( auto& key : keys_to_remove ) {
            m_notification_map.erase(key);
        }
        return TAI_STATUS_SUCCESS;
    }

    tai_status_t PlatformAdapter::get_mux_attribute(const tai_object_type_t& type, const tai_object_id_t& id, tai_attribute_t* const attr) {
        S_ModuleAdapter adapter;
        tai_object_id_t real_id;
        if ( get_mapping(id, &adapter, &real_id) != 0 ) {
            return TAI_STATUS_FAILURE;
        }
        switch (type) {
        case TAI_OBJECT_TYPE_MODULE:
            switch (attr->id) {
            case TAI_MODULE_ATTR_MUX_PLATFORM_ADAPTER_TYPE:
                attr->value.u32 = this->type();
                break;
            case TAI_MODULE_ATTR_MUX_CURRENT_LOADED_TAI_LIBRARY:
                {
                    auto n = adapter->name();
                    auto v = attr->value.charlist.count;
                    attr->value.charlist.count = n.size() + 1;
                    if ( v < (n.size() + 1) ) {
                        return TAI_STATUS_BUFFER_OVERFLOW;
                    }
                    std::strncpy(attr->value.charlist.list, n.c_str(), v);
                    break;
                }
            case TAI_MODULE_ATTR_MUX_REAL_OID:
                attr->value.oid = real_id;
                break;
            default:
                return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
            }
            break;
        case TAI_OBJECT_TYPE_NETWORKIF:
            switch (attr->id) {
            case TAI_NETWORK_INTERFACE_ATTR_MUX_REAL_OID:
                attr->value.oid = real_id;
                break;
            default:
                return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
            }
            break;
        case TAI_OBJECT_TYPE_HOSTIF:
            switch (attr->id) {
            case TAI_HOST_INTERFACE_ATTR_MUX_REAL_OID:
                attr->value.oid = real_id;
                break;
            default:
                return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
            }
            break;
        default:
            return TAI_STATUS_NOT_SUPPORTED;
        }
        return TAI_STATUS_SUCCESS;
    }

    tai_status_t PlatformAdapter::set_mux_attribute(const tai_object_type_t& type, const tai_object_id_t& id, const tai_attribute_t* const attribute, tai::framework::FSMState* state) {
        S_ModuleAdapter adapter;
        tai_object_id_t real_id;
        if ( get_mapping(id, &adapter, &real_id) != 0 ) {
            return TAI_STATUS_FAILURE;
        }
        return TAI_STATUS_NOT_SUPPORTED;
    }


}
