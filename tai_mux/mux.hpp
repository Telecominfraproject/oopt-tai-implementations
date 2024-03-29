#ifndef __MUX_HPP__
#define __MUX_HPP__

#include <string>
#include "platform_adapter.hpp"
#include "module_adapter.hpp"
#include "tai.h"
#include <mutex>

#include "platform.hpp"

namespace tai::mux {

    using namespace tai::framework;
    using log_setting = std::map<tai_api_t, std::pair<tai_log_level_t, tai_log_fn>>;

    class Platform : public tai::framework::Platform {
        public:
            Platform(const tai_service_method_table_t * services);
            tai_status_t create(tai_object_type_t type, tai_object_id_t module_id, uint32_t count, const tai_attribute_t * const list, tai_object_id_t *id);
            tai_status_t remove(tai_object_id_t id);
            tai_object_type_t get_object_type(tai_object_id_t id);
            tai_object_id_t   get_module_id(tai_object_id_t id);
            tai_status_t      set_log(tai_api_t tai_api_id, tai_log_level_t log_level, tai_log_fn log_fn);

            tai_status_t list_metadata(const tai_metadata_key_t *const key, uint32_t *count, const tai_attr_metadata_t *const **list);
            const tai_attr_metadata_t* get_attr_metadata(const tai_metadata_key_t *const key, tai_attr_id_t attr_id);
            const tai_object_type_info_t* get_object_info(const tai_metadata_key_t *const key);
        private:

            tai_status_t get_ma_and_meta_key(const tai_metadata_key_t *const key, tai_metadata_key_t& new_key, S_ModuleAdapter *ma);

            S_PlatformAdapter m_pa;
            log_setting m_log_setting;
    };

    class Module;
    class NetIf;
    class HostIf;

    using S_Module = std::shared_ptr<Module>;
    using S_NetIf  = std::shared_ptr<NetIf>;
    using S_HostIf = std::shared_ptr<HostIf>;

    using S_ConstModule = std::shared_ptr<const Module>;
    using S_ConstNetIf  = std::shared_ptr<const NetIf>;
    using S_ConstHostIf = std::shared_ptr<const HostIf>;

    struct context {
        S_PlatformAdapter pa;
        tai_object_id_t oid;
        tai_object_type_t type;
    };

    tai_status_t attribute_getter(tai_attribute_t* const attribute, void* user);
    tai_status_t attribute_setter(const tai_attribute_t* const attribute, FSMState* state, void* user);

    template<tai_object_type_t T>
    class Object : public tai::framework::Object<T> {
        public:
            Object(S_PlatformAdapter pa) : tai::framework::Object<T>(0, nullptr, std::make_shared<tai::framework::FSM>(),
                    reinterpret_cast<void*>(&m_context),
                    std::bind(&Object::default_setter, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                    std::bind(&Object::default_getter, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                    std::bind(&Object::default_cap_getter, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)),
                    m_context{pa} {}

            tai_object_id_t id() const {
                return m_context.oid;
            }

            tai_object_id_t real_id() const {
                return m_real_id;
            }

        protected:
            tai_object_id_t m_real_id;
            context m_context;
            S_ModuleAdapter m_adapter;
        private:
            tai_status_t default_setter(uint32_t count, const tai_attribute_t* const attribute, FSMState* fsm, void* const user, const tai::framework::error_info* const info) {
                return m_context.pa->set(T, id(), count, attribute);
            }
            tai_status_t default_getter(uint32_t count, tai_attribute_t* const attribute, void* const user, const tai::framework::error_info* const info) {
                return m_context.pa->get(T, id(), count, attribute);
            }
            tai_status_t default_cap_getter(uint32_t count, tai_attribute_capability_t* const caps, void* const user, const tai::framework::error_info* const info) {
                return m_context.pa->get_capability(T, id(), count, caps);
            }
    };

    class Module : public Object<TAI_OBJECT_TYPE_MODULE> {
        public:
            Module(uint32_t count, const tai_attribute_t *list, S_PlatformAdapter platform, const log_setting& log_setting);

            S_ModuleAdapter adapter() const {
                return m_adapter;
            }

            tai_status_t remove() {
                return m_adapter->remove_module(m_real_id);
            }
    };

    class NetIf : public Object<TAI_OBJECT_TYPE_NETWORKIF> {
        public:
            NetIf(S_Module Module, uint32_t count, const tai_attribute_t *list, S_PlatformAdapter platform);
            tai_status_t remove() {
                return m_adapter->remove_network_interface(m_real_id);
            }
            tai_object_id_t module_id() {
                return m_module->id();
            }
        private:
            const S_ConstModule m_module;
    };

    class HostIf : public Object<TAI_OBJECT_TYPE_HOSTIF> {
        public:
            HostIf(S_Module Module, uint32_t count, const tai_attribute_t *list, S_PlatformAdapter platform);
            tai_status_t remove() {
                return m_adapter->remove_host_interface(m_real_id);
            }
            tai_object_id_t module_id() {
                return m_module->id();
            }
        private:
            const S_ConstModule m_module;
    };

};

#ifdef TAI_EXPOSE_PLATFORM
using tai::mux::Platform;
#endif

#endif
