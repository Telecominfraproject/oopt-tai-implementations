#ifndef __MUX_HPP__
#define __MUX_HPP__

#include <string>
#include <exception>
#include "platform_adapter.hpp"
#include "static_platform_adapter.hpp"
#include "module_adapter.hpp"
#include "tai.h"
#include <sstream>
#include <bitset>

class OIDAllocator {
    public:
        tai_object_id_t next() {
            for (int i = 0; i < 256; i++) {
                if ( !m_bitset.test(i) ) {
                    m_bitset.set(i);
                    return tai_object_id_t(i);
                }
            }
            throw std::runtime_error("OID max reach");
        }
        void free(tai_object_id_t oid) {
            m_bitset.reset(oid);
        }
    private:
        std::bitset<256> m_bitset;
};

// singleton
class Multiplexier {
    public:
        Multiplexier(platform_adapter_t pa_kind, uint64_t flags, const tai_service_method_table_t* services) {
            switch ( pa_kind ) {
            case PLATFORM_ADAPTER_STATIC:
                m_pa = new StaticPlatformAdapter(flags, services);
                break;
            default:
                std::stringstream ss;
                ss << "unsupported platform_adapter :" << pa_kind;
                throw std::runtime_error(ss.str());
            }
        }

        ~Multiplexier(){
            if ( m_pa != nullptr ) {
                delete m_pa;
            }
        }

        ModuleAdapter * get_module_adapter(const std::string& location) {
            return m_pa->get_module_adapter(location);
        }

        tai_status_t create_module(
                _Out_ tai_object_id_t *module_id,
                _In_ uint32_t attr_count,
                _In_ const tai_attribute_t *attr_list,
                _In_ tai_module_notification_t *notifications);

        tai_status_t remove_module(
                _In_ tai_object_id_t module_id);

        tai_status_t set_module_attributes(
                _In_ tai_object_id_t        module_id,
                _In_ uint32_t               attr_count,
                _In_ const tai_attribute_t *attr_list);

        tai_status_t get_module_attributes(
                _In_ tai_object_id_t        module_id,
                _In_ uint32_t               attr_count,
                _Out_ tai_attribute_t *attr_list);

        tai_status_t create_network_interface(
                _Out_ tai_object_id_t *network_interface_id,
                _In_ tai_object_id_t module_id,
                _In_ uint32_t attr_count,
                _In_ const tai_attribute_t *attr_list);

        tai_status_t remove_network_interface(
                _In_ tai_object_id_t network_interface_id);

        tai_status_t set_network_interface_attributes(
                _In_ tai_object_id_t        network_interface_id,
                _In_ uint32_t               attr_count,
                _In_ const tai_attribute_t *attr_list);

        tai_status_t get_network_interface_attributes(
                _In_ tai_object_id_t        network_interface_id,
                _In_ uint32_t               attr_count,
                _Out_ tai_attribute_t *attr_list);

        tai_status_t create_host_interface(
                _Out_ tai_object_id_t *host_interface_id,
                _In_ tai_object_id_t module_id,
                _In_ uint32_t attr_count,
                _In_ const tai_attribute_t *attr_list);

        tai_status_t remove_host_interface(
                _In_ tai_object_id_t host_interface_id);

        tai_status_t set_host_interface_attributes(
                _In_ tai_object_id_t        host_interface_id,
                _In_ uint32_t               attr_count,
                _In_ const tai_attribute_t *attr_list);

        tai_status_t get_host_interface_attributes(
                _In_ tai_object_id_t        host_interface_id,
                _In_ uint32_t               attr_count,
                _Out_ tai_attribute_t *attr_list);

    private:
        Multiplexier(const Multiplexier&){}
        void operator = (const Multiplexier&){}
        PlatformAdapter *m_pa;
        std::map<tai_object_id_t, std::pair<tai_object_id_t, ModuleAdapter* >> m_map;
        OIDAllocator m_oid_allocator;

        int get_mapping(const tai_object_id_t& id, ModuleAdapter **adapter, tai_object_id_t *real_id) {
            if ( m_map.find(id) == m_map.end() ) {
                return -1;
            }
            auto pair = m_map[id];
            *adapter = pair.second;
            *real_id = pair.first;
            return 0;
        };

        int create_mapping(tai_object_id_t *id, ModuleAdapter* adapter, const tai_object_id_t& real_id) {
            auto value = std::pair<tai_object_id_t, ModuleAdapter*>(real_id, adapter);
            *id = m_oid_allocator.next();
            m_map[*id] = value;
            return 0;
        }

        int remove_mapping(tai_object_id_t id) {
            if ( m_map.find(id) == m_map.end() ) {
                return 0;
            }
            m_map.erase(id);
            m_oid_allocator.free(id);
            return 0;
        }
};

Multiplexier* create_mux(platform_adapter_t pa_kind, uint64_t flags, const tai_service_method_table_t* services);
#endif
