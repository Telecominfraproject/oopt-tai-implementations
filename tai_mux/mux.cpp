#include "mux.hpp"
#include <iostream>
#include "taimetadata.h"
#include "static_platform_adapter.hpp"
#include "exec_platform_adapter.hpp"

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
        std::string pa_name = DEFAULT_PLATFORM_ADAPTER;
        if (pa) {
            pa_name = std::string(pa);
        }
        if (pa_name == "static") {
            m_pa = std::make_shared<StaticPlatformAdapter>(0, services);
        } else if (pa_name == "exec") {
            m_pa = std::make_shared<ExecPlatformAdapter>(0, services);
        } else {
            TAI_ERROR("unsupported platform_adapter: %s", pa_name.c_str());
            throw Exception(TAI_STATUS_NOT_SUPPORTED);
        }
    }

    tai_status_t Platform::create(tai_object_type_t type, tai_object_id_t module_id, uint32_t count, const tai_attribute_t * const list, tai_object_id_t *id) {
        std::shared_ptr<tai::framework::BaseObject> obj;
        try {
            switch (type) {
            case TAI_OBJECT_TYPE_MODULE:
                obj = std::make_shared<Module>(count, list, m_pa, m_log_setting);
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
        default:
            return TAI_NULL_OBJECT_ID;
        }
    }

    tai_status_t Platform::set_log(tai_api_t api, tai_log_level_t level, tai_log_fn log_fn) {
        auto ret = tai::Logger::get_instance().set_log(api, level, log_fn);
        if ( ret != TAI_STATUS_SUCCESS ) {
            return ret;
        }
        auto set = m_pa->list_module_adapters();
        for ( const auto& a :  set ) {
            auto ret = a->tai_log_set(api, level, log_fn);
            if ( ret != TAI_STATUS_SUCCESS ) {
                return ret;
            }
        }
        m_log_setting[api] = {level, log_fn};
        return TAI_STATUS_SUCCESS;
    }

    tai_status_t Platform::get_ma_and_meta_key(const tai_metadata_key_t *const key, tai_metadata_key_t& new_key, S_ModuleAdapter *ma) {
        new_key = *key;
        if ( key->location.count > 0 ) {
            std::string loc(key->location.list, key->location.count);
            *ma = m_pa->get_module_adapter(loc);
        } else if ( key->oid != TAI_NULL_OBJECT_ID ) {
            tai_object_id_t real_id;
            if ( m_pa->get_mapping(key->oid, ma, &real_id) < 0 ) {
                return TAI_STATUS_FAILURE;
            }
            new_key.oid = real_id;
        }
        return TAI_STATUS_SUCCESS;
    }

    tai_status_t Platform::list_metadata(const tai_metadata_key_t *const key, uint32_t *count, const tai_attr_metadata_t *const **list) {
        S_ModuleAdapter ma;
        tai_metadata_key_t new_key;
        auto ret = get_ma_and_meta_key(key, new_key, &ma);
        if ( ret != TAI_STATUS_SUCCESS ) {
            return ret;
        }

        if ( ma ) {
            return ma->list_metadata(&new_key, count, list);
        }

        auto type = key->type;
        auto info = tai_metadata_all_object_type_infos[type];
        if ( info == nullptr ) {
            *count = tai_metadata_attr_sorted_by_id_name_count;
            *list = tai_metadata_attr_sorted_by_id_name;
            return TAI_STATUS_SUCCESS;
        }
        *count = info->attrmetadatalength;
        *list = info->attrmetadata;
        return TAI_STATUS_SUCCESS;
    }

    const tai_attr_metadata_t* Platform::get_attr_metadata(const tai_metadata_key_t *const key, tai_attr_id_t attr_id) {
        S_ModuleAdapter ma;
        tai_metadata_key_t new_key;
        auto ret = get_ma_and_meta_key(key, new_key, &ma);
        if ( ret != TAI_STATUS_SUCCESS ) {
            return nullptr;
        }
        if ( ma ) {
            return ma->get_attr_metadata(&new_key, attr_id);
        }
        return tai_metadata_get_attr_metadata(new_key.type, attr_id);
    }

    const tai_object_type_info_t* Platform::get_object_info(const tai_metadata_key_t *const key) {
        S_ModuleAdapter ma;
        tai_metadata_key_t new_key;
        auto ret = get_ma_and_meta_key(key, new_key, &ma);
        if ( ret != TAI_STATUS_SUCCESS ) {
            return nullptr;
        }
        if ( ma ) {
            return ma->get_object_info(&new_key);
        }
        return tai_metadata_get_object_type_info(key->type);
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

    Module::Module(uint32_t count, const tai_attribute_t *list, S_PlatformAdapter platform, const log_setting& log_setting) : Object(platform) {
        auto mod_addr = find_attribute_in_list(TAI_MODULE_ATTR_LOCATION, count, list);
        if ( mod_addr == nullptr ) {
            throw Exception(TAI_STATUS_MANDATORY_ATTRIBUTE_MISSING);
        }
        const std::string location(mod_addr->charlist.list, mod_addr->charlist.count);
        auto adapter = m_context.pa->get_module_adapter(location);
        if ( adapter == nullptr ) {
            throw Exception(TAI_STATUS_FAILURE);
        }
        for (auto const& v : log_setting) {
            auto ret = adapter->tai_log_set(v.first, v.second.first, v.second.second);
            if ( ret != TAI_STATUS_SUCCESS ) {
                throw Exception(ret);
            }
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

    NetIf::NetIf(S_Module module, uint32_t count, const tai_attribute_t *list, S_PlatformAdapter platform) : Object(platform), m_module(module) {
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

    HostIf::HostIf(S_Module module, uint32_t count, const tai_attribute_t *list, S_PlatformAdapter platform) : Object(platform), m_module(module) {
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
