#ifndef __MUX_HPP__
#define __MUX_HPP__

#include <string>
#include "platform_adapter.hpp"
#include "static_platform_adapter.hpp"
#include "module_adapter.hpp"
#include "tai.h"
#include <mutex>

#include "platform.hpp"

namespace tai::mux {

    class Platform : public tai::Platform {
        public:
            Platform(const tai_service_method_table_t * services);
            tai_status_t create(tai_object_type_t type, tai_object_id_t module_id, uint32_t count, const tai_attribute_t * const list, tai_object_id_t *id);
            tai_status_t remove(tai_object_id_t id);
            tai_object_type_t get_object_type(tai_object_id_t id);
            tai_object_id_t   get_module_id(tai_object_id_t id);
            tai_status_t      set_log(tai_api_t tai_api_id, tai_log_level_t log_level, tai_log_fn log_fn);
        private:
            S_PlatformAdapter m_pa;
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

    template<tai_object_type_t T>
    class Object : public tai::Object<T> {
        public:
            Object(S_PlatformAdapter pa) : m_pa(pa), tai::Object<T>(0, nullptr, std::make_shared<tai::FSM>(), nullptr, std::bind(&Object::setter, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), std::bind(&Object::getter, this, std::placeholders::_1, std::placeholders::_2)) {}
            tai_object_id_t id() const {
                return m_id;
            }

            tai_object_id_t real_id() const {
                return m_real_id;
            }

        protected:
            tai_object_id_t m_id, m_real_id;
            S_PlatformAdapter m_pa;
            S_ModuleAdapter m_adapter;
        private:
            tai_status_t setter(const tai_attribute_t* const attribute, FSMState* fsm, void* const user) {
                return m_pa->set(T, id(), attribute);
            }
            tai_status_t getter(tai_attribute_t* const attribute, void* const user) {
                auto ret = m_adapter->get_attributes(T, m_real_id, 1, attribute);
                if ( ret != TAI_STATUS_SUCCESS ) {
                    return ret;
                }
                return m_pa->convert_oid(T, id(), attribute, attribute, true);
            }
    };

    class Module : public Object<TAI_OBJECT_TYPE_MODULE> {
        public:
            Module(uint32_t count, const tai_attribute_t *list, S_PlatformAdapter platform);

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
