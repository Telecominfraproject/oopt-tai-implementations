#ifndef __PLATFORM_ADAPTER_HPP__
#define __PLATFORM_ADAPTER_HPP__

#include <string>
#include <unordered_set>
#include <memory>
#include <bitset>
#include <map>
#include <mutex>

#include "tai.h"
#include "attribute.hpp"
#include "logger.hpp"

#include "fsm.hpp"

namespace tai::mux {

    class ModuleAdapter;
    using S_ModuleAdapter = std::shared_ptr<ModuleAdapter>;

    class PlatformAdapter;
    using S_PlatformAdapter = std::shared_ptr<PlatformAdapter>;

    using notification_key = std::pair<tai_object_id_t, tai_attr_id_t>;

    struct NotificationContext {
        PlatformAdapter *pa;
        tai_notification_handler_t real_handler;
        tai_object_id_t muxed_oid;
        tai_object_type_t object_type;
        std::mutex mutex;
    };

    using S_NotificationContext = std::shared_ptr<NotificationContext>;

    static const int TAI_MUX_NUM_MAX_OBJECT = 256;

    class OIDAllocator {
        public:
            tai_object_id_t next() {
                // TAI_NULL_OBJECT_ID == 0, start from i = 1
                for (int i = 1; i < TAI_MUX_NUM_MAX_OBJECT; i++) {
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
            std::bitset<TAI_MUX_NUM_MAX_OBJECT> m_bitset;
    };

    class PlatformAdapter {
        public:
            virtual S_ModuleAdapter get_module_adapter(const std::string& location) = 0;

            /** @brief return the set of loaded module adapters. */
            virtual const std::unordered_set<S_ModuleAdapter> list_module_adapters() = 0;
            PlatformAdapter(){}
            virtual ~PlatformAdapter(){}

            virtual tai_mux_platform_adapter_type_t type() const = 0;

            int get_mapping(const tai_object_id_t& id, S_ModuleAdapter *adapter, tai_object_id_t *real_id) {
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

            tai_object_id_t get_reverse_mapping(const tai_object_id_t real_id, S_ModuleAdapter adapter) {
                for ( auto p : m_map ) {
                    if ( p.second.first == real_id && p.second.second == adapter ) {
                        return p.first;
                    }
                }
                return TAI_NULL_OBJECT_ID;
            }

            int create_mapping(tai_object_id_t *id, S_ModuleAdapter adapter, const tai_object_id_t& real_id) {
                auto value = std::pair<tai_object_id_t, S_ModuleAdapter>(real_id, adapter);
                *id = m_oid_allocator.next();
                m_map[*id] = value;
                return 0;
            }

            virtual int remove_mapping(tai_object_id_t id) {
                if ( m_map.find(id) == m_map.end() ) {
                    return 0;
                }
                m_map.erase(id);
                m_oid_allocator.free(id);
                return 0;
            }

            void notify(NotificationContext* ctx, tai_object_id_t real_oid, uint32_t attr_count, tai_attribute_t const * const attr_list);

            tai_status_t convert_oid(const tai_object_type_t& type, const tai_object_id_t& id, const S_ConstAttribute src, const S_Attribute dst, bool reversed);
            tai_status_t convert_oid(const tai_object_type_t& type, const tai_object_id_t& id, const tai_attribute_t * const src, tai_attribute_t * const dst, bool reversed);

            tai_status_t get(const tai_object_type_t& type, const tai_object_id_t& id, uint32_t count, tai_attribute_t* const attrs);
            tai_status_t set(const tai_object_type_t& type, const tai_object_id_t& id, uint32_t count, const tai_attribute_t* const attrs);

            virtual tai_status_t get_mux_attribute(const tai_object_type_t& type, const tai_object_id_t& oid, tai_attribute_t* const attribute);
            virtual tai_status_t set_mux_attribute(const tai_object_type_t& type, const tai_object_id_t& oid, const tai_attribute_t* const attribute, tai::framework::FSMState* state);

        private:
            PlatformAdapter(const PlatformAdapter&){}
            void operator = (const PlatformAdapter&){}
            OIDAllocator m_oid_allocator;
            std::map<tai_object_id_t, std::pair<tai_object_id_t, S_ModuleAdapter>> m_map;
            std::map<notification_key, S_NotificationContext> m_notification_map;
    };

    using S_PlatformAdapter = std::shared_ptr<PlatformAdapter>;

};

#endif
