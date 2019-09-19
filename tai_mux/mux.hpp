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
#include <mutex>

class OIDAllocator {
    public:
        tai_object_id_t next() {
            // TAI_NULL_OBJECT_ID == 0, start from i = 1
            for (int i = 1; i < 256; i++) {
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

class Multiplexier;

struct notification_context {
    Multiplexier* mux;
    ModuleAdapter* adapter;
    tai_notification_handler_t handler;
    tai_attr_id_t notify_id;
    std::mutex mutex;
};

using notification_key = std::pair<tai_object_id_t, tai_attr_id_t>;

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
            for ( auto& n : m_notification_map ) {
                delete n.second;
            }
        }

        ModuleAdapter * get_module_adapter(const std::string& location) {
            return m_pa->get_module_adapter(location);
        }

        void notify(notification_context* ctx, tai_object_id_t real_oid, uint32_t attr_count, tai_attribute_t const * const attr_list);

        tai_status_t create_module(
                _Out_ tai_object_id_t *module_id,
                _In_ uint32_t attr_count,
                _In_ const tai_attribute_t *attr_list);

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

        tai_status_t clear_host_interface_attributes(
                _In_ tai_object_id_t        host_interface_id,
                _In_ uint32_t               attr_count,
                _Out_ tai_attr_id_t *attr_list);

        tai_object_type_t object_type_query(
                _In_ tai_object_id_t id);

        tai_object_id_t module_id_query(
                _In_ tai_object_id_t id);

        tai_status_t tai_log_set(
                _In_ tai_api_t tai_api_id,
                _In_ tai_log_level_t log_level,
                _In_ tai_log_fn log_fn);

    private:
        Multiplexier(const Multiplexier&){}
        void operator = (const Multiplexier&){}
        PlatformAdapter *m_pa;
        std::map<tai_object_id_t, std::pair<tai_object_id_t, ModuleAdapter* >> m_map;
        std::map<notification_key, notification_context*> m_notification_map;

        OIDAllocator m_oid_allocator;

        tai_status_t set_attributes( std::function<tai_status_t(ModuleAdapter*, tai_object_id_t, uint32_t, const tai_attribute_t*)> f, tai_object_id_t oid, uint32_t attr_count, const tai_attribute_t *attr_list);
        tai_status_t get_attributes( std::function<tai_status_t(ModuleAdapter*, tai_object_id_t, uint32_t, tai_attribute_t*)> f, tai_object_id_t oid, uint32_t attr_count, tai_attribute_t *attr_list);

        int get_mapping(const tai_object_id_t& id, ModuleAdapter **adapter, tai_object_id_t *real_id) {
            if ( m_map.find(id) == m_map.end() ) {
                return -1;
            }
            auto pair = m_map[id];
            if ( adapter != nullptr ) {
                *adapter = pair.second;
            }
            if ( real_id != nullptr ) {
                *real_id = pair.first;
            }
            return 0;
        };

        tai_object_id_t get_reverse_mapping(const tai_object_id_t real_id, ModuleAdapter *adapter) {
            for ( auto p : m_map ) {
                if ( p.second.first == real_id && p.second.second == adapter ) {
                    return p.first;
                }
            }
            return TAI_NULL_OBJECT_ID;
        }

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

        tai_status_t convert_oid(tai_object_id_t oid, const tai_attribute_t *src, tai_attribute_t *dst, bool reversed);
        int free_attributes(tai_object_type_t t, std::vector<tai_attribute_t>& attributes);
};

Multiplexier* create_mux(platform_adapter_t pa_kind, uint64_t flags, const tai_service_method_table_t* services);
#endif
