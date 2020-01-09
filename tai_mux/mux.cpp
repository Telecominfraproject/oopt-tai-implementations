#include "mux.hpp"
#include <iostream>
#include "taimetadata.h"

namespace tai::mux {

    static const std::string PLATFORM_ADAPTER = "TAI_MUX_PLATFORM_ADAPTER";

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

#ifndef DEFAULT_PLATFORM_ADAPTER
#define DEFAULT_PLATFORM_ADAPTER "static"
#endif

    Platform::Platform(const tai_service_method_table_t * services) : tai::framework::Platform(services) {
        auto pa = std::getenv(PLATFORM_ADAPTER.c_str());
        std::string pa_name;
        if ( pa == nullptr ) {
            pa_name = DEFAULT_PLATFORM_ADAPTER;
        } else {
            pa_name = std::string(pa);
        }
        tai_mux_platform_adapter_type_t pa_type;
        if ( pa_name == "static" ) {
            pa_type = TAI_MUX_PLATFORM_ADAPTER_TYPE_STATIC;
        }
        switch ( pa_type ) {
        case TAI_MUX_PLATFORM_ADAPTER_TYPE_STATIC:
            m_pa = std::make_shared<StaticPlatformAdapter>(0, services);
            break;
        default:
            ERROR("unsupported platform_adapter: %s", pa_name.c_str());
            throw Exception(TAI_STATUS_NOT_SUPPORTED);
        }
    }

    tai_status_t Platform::create(tai_object_type_t type, tai_object_id_t module_id, uint32_t count, const tai_attribute_t * const list, tai_object_id_t *id) {
        std::shared_ptr<tai::framework::BaseObject> obj;
        try {
            switch (type) {
            case TAI_OBJECT_TYPE_MODULE:
                obj = std::make_shared<Module>(count, list, m_pa);
                break;
            case TAI_OBJECT_TYPE_NETWORKIF:
            case TAI_OBJECT_TYPE_HOSTIF:
                {
                    auto it = m_objects.find(module_id);
                    if ( it == m_objects.end() ) {
                        return TAI_STATUS_UNINITIALIZED;
                    }
                    if ( it->second->type() != TAI_OBJECT_TYPE_MODULE ) {
                        return TAI_STATUS_INVALID_OBJECT_ID;
                    }
                    auto module = std::dynamic_pointer_cast<Module>(it->second);
                    if ( type == TAI_OBJECT_TYPE_NETWORKIF ) {
                        obj = std::make_shared<NetIf>(module, count, list, m_pa);
                    } else {
                        obj = std::make_shared<HostIf>(module, count, list, m_pa);
                    }
                }
                break;
            default:
                return TAI_STATUS_NOT_SUPPORTED;
            }
        } catch (Exception& e) {
            return e.err();
        } catch (...) {
            return TAI_STATUS_FAILURE;
        }

        auto oid = obj->id();
        auto it = m_objects.find(oid);
        if ( it != m_objects.end() ) {
            return TAI_STATUS_ITEM_ALREADY_EXISTS;
        }
        m_objects[oid] = obj;
        *id = oid;
        return TAI_STATUS_SUCCESS;
    }

    tai_status_t Platform::remove(tai_object_id_t id) {
        auto it = m_objects.find(id);
        if ( it == m_objects.end() ) {
            return TAI_STATUS_ITEM_NOT_FOUND;
        }
        auto type = get_object_type(id);
        tai_status_t ret;
        switch (type) {
        case TAI_OBJECT_TYPE_MODULE:
            {
                auto m = std::dynamic_pointer_cast<Module>(it->second);
                ret = m->remove();
            }
            break;
        case TAI_OBJECT_TYPE_NETWORKIF:
            {
                auto m = std::dynamic_pointer_cast<NetIf>(it->second);
                ret = m->remove();
            }
            break;
        case TAI_OBJECT_TYPE_HOSTIF:
            {
                auto m = std::dynamic_pointer_cast<HostIf>(it->second);
                ret = m->remove();
            }
            break;
        default:
            ret = TAI_STATUS_INVALID_OBJECT_ID;
        }

        if ( ret != TAI_STATUS_SUCCESS ) {
            return ret;
        }
        m_objects.erase(it);
        m_pa->remove_mapping(id);
        return TAI_STATUS_SUCCESS;
    }

    tai_object_type_t Platform::get_object_type(tai_object_id_t id) {
        auto it = m_objects.find(id);
        if ( it == m_objects.end() ) {
            return TAI_OBJECT_TYPE_NULL;
        }
        return it->second->type();
    }

    tai_object_id_t Platform::get_module_id(tai_object_id_t id) {
        auto it = m_objects.find(id);
        if ( it == m_objects.end() ) {
            return TAI_NULL_OBJECT_ID;
        }
        switch (it->second->type()) {
        case TAI_OBJECT_TYPE_MODULE:
            {
                auto m = std::dynamic_pointer_cast<Module>(it->second);
                return m->id();
            }
        case TAI_OBJECT_TYPE_NETWORKIF:
            {
                auto m = std::dynamic_pointer_cast<NetIf>(it->second);
                return m->module_id();
            }
        case TAI_OBJECT_TYPE_HOSTIF:
            {
                auto m = std::dynamic_pointer_cast<HostIf>(it->second);
                return m->module_id();
            }
        }
        return TAI_NULL_OBJECT_ID;
    }

    tai_status_t Platform::set_log(tai_api_t api, tai_log_level_t level, tai_log_fn log_fn) {
        auto set = m_pa->list_module_adapters();
        for ( const auto& a :  set ) {
            auto ret = a->tai_log_set(api, level, log_fn);
            if ( ret != TAI_STATUS_SUCCESS ) {
                return ret;
            }
        }
        return TAI_STATUS_SUCCESS;
    }

    tai_status_t attribute_getter(tai_attribute_t* const attribute, void* user) {
        auto ctx = reinterpret_cast<context*>(user);
        auto pa = ctx->pa;
        return pa->get_mux_attribute(ctx->type, ctx->oid, attribute);
    }

    tai_status_t attribute_setter(const tai_attribute_t* const attribute, FSMState* state, void* user) {
        auto ctx = reinterpret_cast<context*>(user);
        auto pa = ctx->pa;
        return pa->set_mux_attribute(ctx->type, ctx->oid, attribute, state);
    }

    using M = AttributeInfo<TAI_OBJECT_TYPE_MODULE>;
    using N = AttributeInfo<TAI_OBJECT_TYPE_NETWORKIF>;
    using H = AttributeInfo<TAI_OBJECT_TYPE_HOSTIF>;

    template <> const AttributeInfoMap<TAI_OBJECT_TYPE_MODULE> Config<TAI_OBJECT_TYPE_MODULE>::m_info {
        mux::M(TAI_MODULE_ATTR_MUX_PLATFORM_ADAPTER_TYPE)
            .set_getter(&mux::attribute_getter),
        mux::M(TAI_MODULE_ATTR_MUX_CURRENT_LOADED_TAI_LIBRARY)
            .set_getter(&mux::attribute_getter),
        mux::M(TAI_MODULE_ATTR_MUX_REAL_OID)
            .set_getter(&mux::attribute_getter),
    };

    Module::Module(uint32_t count, const tai_attribute_t *list, S_PlatformAdapter platform) : Object(platform) {
        auto mod_addr = find_attribute_in_list(TAI_MODULE_ATTR_LOCATION, count, list);
        if ( mod_addr == nullptr ) {
            throw Exception(TAI_STATUS_MANDATORY_ATTRIBUTE_MISSING);
        }
        const std::string location(mod_addr->charlist.list, mod_addr->charlist.count);
        auto adapter = m_context.pa->get_module_adapter(location);
        if ( adapter == nullptr ) {
            throw Exception(TAI_STATUS_FAILURE);
        }
        m_adapter = adapter;
        auto ret = m_adapter->create_module(&m_real_id, count, list);
        if ( ret != TAI_STATUS_SUCCESS ) {
            throw Exception(ret);
        }
        if ( platform->create_mapping(&m_context.oid, m_adapter, m_real_id) != 0 ) {
            throw Exception(TAI_STATUS_FAILURE);
        }
        m_context.type = TAI_OBJECT_TYPE_MODULE;
    }

    template <> const AttributeInfoMap<TAI_OBJECT_TYPE_NETWORKIF> Config<TAI_OBJECT_TYPE_NETWORKIF>::m_info {
        mux::N(TAI_NETWORK_INTERFACE_ATTR_MUX_REAL_OID)
            .set_getter(&mux::attribute_getter),
    };

    NetIf::NetIf(S_Module module, uint32_t count, const tai_attribute_t *list, S_PlatformAdapter platform) : m_module(module), Object(platform) {
        m_adapter = module->adapter();
        if ( m_adapter == nullptr ) {
            throw Exception(TAI_STATUS_FAILURE);
        }
        auto ret = m_adapter->create_network_interface(&m_real_id, module->real_id(), count, list);
        if ( ret != TAI_STATUS_SUCCESS ) {
            throw Exception(ret);
        }
        if ( platform->create_mapping(&m_context.oid, m_adapter, m_real_id) != 0 ) {
            throw Exception(TAI_STATUS_FAILURE);
        }
        m_context.type = TAI_OBJECT_TYPE_NETWORKIF;
    }

    template <> const AttributeInfoMap<TAI_OBJECT_TYPE_HOSTIF> Config<TAI_OBJECT_TYPE_HOSTIF>::m_info {
        mux::H(TAI_HOST_INTERFACE_ATTR_MUX_REAL_OID)
            .set_getter(&mux::attribute_getter),
    };

    HostIf::HostIf(S_Module module, uint32_t count, const tai_attribute_t *list, S_PlatformAdapter platform) : m_module(module), Object(platform) {
        m_adapter = module->adapter();
        if ( m_adapter == nullptr ) {
            throw Exception(TAI_STATUS_FAILURE);
        }
        auto ret = m_adapter->create_host_interface(&m_real_id, module->real_id(), count, list);
        if ( ret != TAI_STATUS_SUCCESS ) {
            throw Exception(ret);
        }
        if ( platform->create_mapping(&m_context.oid, m_adapter, m_real_id) != 0 ) {
            throw Exception(TAI_STATUS_FAILURE);
        }
        m_context.type = TAI_OBJECT_TYPE_HOSTIF;
    }
};
