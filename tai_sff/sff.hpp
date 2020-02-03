#ifndef __SFF_HPP__
#define __SFF_HPP__

#include "platform.hpp"
#include "logger.hpp"
#include "sff_fsm.hpp"

#include <sys/timerfd.h>
#include <cmath>

namespace tai::sff {

    using namespace tai::framework;

    const uint8_t OBJECT_TYPE_SHIFT = 48;

    class Platform : public tai::framework::Platform {
        public:
            Platform(const tai_service_method_table_t * services);
            tai_status_t create(tai_object_type_t type, tai_object_id_t module_id, uint32_t attr_count, const tai_attribute_t * const attr_list, tai_object_id_t *id);
            tai_status_t remove(tai_object_id_t id) {
                return TAI_STATUS_NOT_SUPPORTED;
            }
            tai_object_type_t get_object_type(tai_object_id_t id);
            tai_object_id_t   get_module_id(tai_object_id_t id);
    };

    struct context {
        S_FSM fsm;
        tai_object_type_t type;
        tai_object_id_t oid;
    };

    template<tai_object_type_t T>
    class Object : public tai::framework::Object<T> {
        public:
            Object(uint32_t count, const tai_attribute_t *list, S_FSM fsm) : m_context{fsm, T}, tai::framework::Object<T>(count, list, fsm, reinterpret_cast<void*>(&m_context)) {}

            tai_object_id_t id() const {
                return m_context.oid;
            }

        protected:
            context m_context;
    };

    class Module : public Object<TAI_OBJECT_TYPE_MODULE> {
        public:
            Module(uint32_t count, const tai_attribute_t *list, S_FSM fsm) : m_fsm(fsm), Object(count, list, fsm) {
                auto loc = fsm->location();
                std::ifstream ifs(loc + "/port_name");
                if ( !ifs ) {
                    throw Exception(TAI_STATUS_ITEM_NOT_FOUND);
                }
                std::string buf;
                ifs >> buf;
                int i = -1;
                std::sscanf(buf.c_str(), "port%d", &i);
                if ( i < 0 ) {
                    TAI_ERROR("failed to parse port_name: %s", buf.c_str());
                    throw Exception(TAI_STATUS_ITEM_NOT_FOUND);
                }
                m_context.oid = static_cast<tai_object_id_t>(uint64_t(TAI_OBJECT_TYPE_MODULE) << OBJECT_TYPE_SHIFT | i);
            }

            S_FSM fsm() {
                return m_fsm;
            }
        private:
            S_FSM m_fsm;
    };

    class NetIf : public Object<TAI_OBJECT_TYPE_NETWORKIF> {
        public:
            NetIf(S_Module module, uint32_t count, const tai_attribute_t *list) : Object(count, list, module->fsm()) {
                int index = -1;
                for ( auto i = 0; i < count; i++ ) {
                    if ( list[i].id == TAI_NETWORK_INTERFACE_ATTR_INDEX ) {
                        index = list[i].value.u32;
                        break;
                    }
                }
                if ( index < 0 ) {
                    throw Exception(TAI_STATUS_MANDATORY_ATTRIBUTE_MISSING);
                }
                m_context.oid = static_cast<tai_object_id_t>(uint64_t(TAI_OBJECT_TYPE_NETWORKIF) << OBJECT_TYPE_SHIFT | (module->id() & 0xff) << 8 | index);
            }
    };

    class HostIf : public Object<TAI_OBJECT_TYPE_HOSTIF> {
        public:
            HostIf(S_Module module, uint32_t count, const tai_attribute_t *list) : Object(count, list, module->fsm()) {
                int index = -1;
                for ( auto i = 0; i < count; i++ ) {
                    if ( list[i].id == TAI_HOST_INTERFACE_ATTR_INDEX ) {
                        index = list[i].value.u32;
                        break;
                    }
                }
                if ( index < 0 ) {
                    throw Exception(TAI_STATUS_MANDATORY_ATTRIBUTE_MISSING);
                }
                m_context.oid = static_cast<tai_object_id_t>(uint64_t(TAI_OBJECT_TYPE_HOSTIF) << OBJECT_TYPE_SHIFT | (module->id() & 0xff) << 8 | index);
            }
    };

};

#ifdef TAI_EXPOSE_PLATFORM
using tai::sff::Platform;
#endif

#endif // __SFF_HPP__
