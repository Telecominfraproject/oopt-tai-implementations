#ifndef __MODULE_ADAPTER_HPP__
#define __MODULE_ADAPTER_HPP__

#include "tai.h"
#include <string>
#include <memory>

namespace tai::mux {

    typedef tai_status_t (*tai_api_initialize_fn) (uint64_t, const tai_service_method_table_t *);
    typedef tai_status_t (*tai_api_uninitialize_fn) (void);
    typedef tai_status_t (*tai_api_query_fn) (tai_api_t, void**);
    typedef tai_status_t (*tai_log_set_fn) (tai_api_t, tai_log_level_t, tai_log_fn);
    typedef tai_object_type_t (*tai_object_type_query_fn) (tai_object_id_t);
    typedef tai_object_id_t (*tai_module_id_query_fn) (tai_object_id_t);

    // corresponds to one dynamic library
    class ModuleAdapter {
        public:
            ModuleAdapter(const std::string& name, uint64_t flags, const tai_service_method_table_t* services);
            ~ModuleAdapter();
            static uint64_t dl_address(const std::string& name);

            std::string name() const {
                return m_name;
            }

            tai_status_t tai_api_initialize(uint64_t flags, const tai_service_method_table_t* services) {
                return m_tai_api_initialize(flags, services);
            }
            tai_status_t tai_api_query(tai_api_t tai_api_id, void** api_method_table) {
                return m_tai_api_query(tai_api_id, api_method_table);
            }
            tai_status_t tai_api_uninitialize(void) {
                return m_tai_api_uninitialize();
            }
            tai_status_t tai_log_set(tai_api_t tai_api_id, tai_log_level_t log_level, tai_log_fn log_fn) {
                return m_tai_log_set(tai_api_id, log_level, log_fn);
            }
            tai_object_type_t tai_object_type_query(tai_object_id_t tai_object_id) {
                return m_tai_object_type_query(tai_object_id);
            }
            tai_object_id_t tai_module_id_query(tai_object_id_t tai_object_id) {
                return m_tai_module_id_query(tai_object_id);
            }

            tai_status_t set_attributes(tai_object_type_t type, tai_object_id_t oid, uint32_t count, const tai_attribute_t *list) {
                switch (type) {
                case TAI_OBJECT_TYPE_MODULE:
                    return set_module_attributes(oid, count, list);
                case TAI_OBJECT_TYPE_NETWORKIF:
                    return set_network_interface_attributes(oid, count, list);
                case TAI_OBJECT_TYPE_HOSTIF:
                    return set_host_interface_attributes(oid, count, list);
                }
                return TAI_STATUS_NOT_SUPPORTED;
            }

            tai_status_t get_attributes(tai_object_type_t type, tai_object_id_t oid, uint32_t count, tai_attribute_t *list) {
                switch (type) {
                case TAI_OBJECT_TYPE_MODULE:
                    return get_module_attributes(oid, count, list);
                case TAI_OBJECT_TYPE_NETWORKIF:
                    return get_network_interface_attributes(oid, count, list);
                case TAI_OBJECT_TYPE_HOSTIF:
                    return get_host_interface_attributes(oid, count, list);
                }
                return TAI_STATUS_NOT_SUPPORTED;
            }

            tai_status_t create_module(
                _Out_ tai_object_id_t          *module_id,
                _In_ uint32_t                   attr_count,
                _In_ const tai_attribute_t     *attr_list) {
                if ( m_module_api == nullptr || m_module_api->create_module == nullptr ) {
                    return TAI_STATUS_FAILURE;
                }
                return m_module_api->create_module(module_id, attr_count, attr_list);
            }

            tai_status_t remove_module(
                _In_ tai_object_id_t module_id) {
                if ( m_module_api == nullptr || m_module_api->remove_module == nullptr ) {
                    return TAI_STATUS_FAILURE;
                }
                return m_module_api->remove_module(module_id);
            }

            tai_status_t set_module_attributes(
                _In_ tai_object_id_t module_id,
                _In_ uint32_t attr_count,
                _In_ const tai_attribute_t *attr_list) {
                if ( m_module_api == nullptr || m_module_api->set_module_attributes == nullptr ) {
                    return TAI_STATUS_FAILURE;
                }
                return m_module_api->set_module_attributes(module_id, attr_count, attr_list);
            }

            tai_status_t get_module_attributes(
                _In_ tai_object_id_t module_id,
                _In_ uint32_t attr_count,
                _In_ tai_attribute_t *attr_list) {
                if ( m_module_api == nullptr || m_module_api->get_module_attributes == nullptr ) {
                    return TAI_STATUS_FAILURE;
                }
                return m_module_api->get_module_attributes(module_id, attr_count, attr_list);
            }

            tai_status_t create_network_interface(
                _Out_ tai_object_id_t *network_interface_id,
                _In_ tai_object_id_t module_id,
                _In_ uint32_t attr_count,
                _In_ const tai_attribute_t *attr_list) {
                if ( m_netif_api == nullptr || m_netif_api->create_network_interface == nullptr ) {
                    return TAI_STATUS_FAILURE;
                }
                return m_netif_api->create_network_interface(network_interface_id, module_id, attr_count, attr_list);
            }

            tai_status_t remove_network_interface(
                _In_ tai_object_id_t network_interface_id) {
                if ( m_netif_api == nullptr || m_netif_api->remove_network_interface == nullptr ) {
                    return TAI_STATUS_FAILURE;
                }
                return m_netif_api->remove_network_interface(network_interface_id);
            }

            tai_status_t set_network_interface_attributes(
                _In_ tai_object_id_t network_interface_id,
                _In_ uint32_t attr_count,
                _In_ const tai_attribute_t *attr_list) {
                if ( m_netif_api == nullptr || m_netif_api->set_network_interface_attributes == nullptr ) {
                    return TAI_STATUS_FAILURE;
                }
                return m_netif_api->set_network_interface_attributes(network_interface_id, attr_count, attr_list);
            }

            tai_status_t get_network_interface_attributes(
                _In_ tai_object_id_t network_interface_id,
                _In_ uint32_t attr_count,
                _In_ tai_attribute_t *attr_list) {
                if ( m_netif_api == nullptr || m_netif_api->get_network_interface_attributes == nullptr ) {
                    return TAI_STATUS_FAILURE;
                }
                return m_netif_api->get_network_interface_attributes(network_interface_id, attr_count, attr_list);
            }

            tai_status_t create_host_interface(
                _Out_ tai_object_id_t *host_interface_id,
                _In_ tai_object_id_t module_id,
                _In_ uint32_t attr_count,
                _In_ const tai_attribute_t *attr_list) {
                if ( m_hostif_api == nullptr || m_hostif_api->create_host_interface == nullptr ) {
                    return TAI_STATUS_FAILURE;
                }
                return m_hostif_api->create_host_interface(host_interface_id, module_id, attr_count, attr_list);
            }

            tai_status_t remove_host_interface(
                _In_ tai_object_id_t host_interface_id) {
                if ( m_hostif_api == nullptr || m_hostif_api->remove_host_interface == nullptr ) {
                    return TAI_STATUS_FAILURE;
                }
                return m_hostif_api->remove_host_interface(host_interface_id);
            }

            tai_status_t set_host_interface_attributes(
                _In_ tai_object_id_t host_interface_id,
                _In_ uint32_t attr_count,
                _In_ const tai_attribute_t *attr_list) {
                if ( m_hostif_api == nullptr || m_hostif_api->set_host_interface_attributes == nullptr ) {
                    return TAI_STATUS_FAILURE;
                }
                return m_hostif_api->set_host_interface_attributes(host_interface_id, attr_count, attr_list);
            }

            tai_status_t get_host_interface_attributes(
                _In_ tai_object_id_t host_interface_id,
                _In_ uint32_t attr_count,
                _In_ tai_attribute_t *attr_list) {
                if ( m_hostif_api == nullptr || m_hostif_api->get_host_interface_attributes == nullptr ) {
                    return TAI_STATUS_FAILURE;
                }
                return m_hostif_api->get_host_interface_attributes(host_interface_id, attr_count, attr_list);
            }

            tai_status_t clear_host_interface_attributes(
                _In_ tai_object_id_t host_interface_id,
                _In_ uint32_t attr_count,
                _In_ tai_attr_id_t *attr_list) {
                if ( m_hostif_api == nullptr || m_hostif_api->clear_host_interface_attributes == nullptr ) {
                    return TAI_STATUS_FAILURE;
                }
                return m_hostif_api->clear_host_interface_attributes(host_interface_id, attr_count, attr_list);
            }

        private:
            void* m_dl;
            const std::string m_name;
            tai_api_initialize_fn    m_tai_api_initialize;
            tai_api_uninitialize_fn  m_tai_api_uninitialize;
            tai_api_query_fn         m_tai_api_query;
            tai_log_set_fn           m_tai_log_set;
            tai_object_type_query_fn m_tai_object_type_query;
            tai_module_id_query_fn   m_tai_module_id_query;

            tai_module_api_t*            m_module_api;
            tai_host_interface_api_t*    m_hostif_api;
            tai_network_interface_api_t* m_netif_api;
    };

    using S_ModuleAdapter = std::shared_ptr<ModuleAdapter>;

};
#endif
