#include <thread>
#include <chrono>
#include <iostream>
#include <vector>
#include <thread>
#include <map>

#include <mutex>
#include <queue>

#include <sys/eventfd.h>

#include "unistd.h"

#include "tai.h"

tai_module_api_t *module_api;
tai_network_interface_api_t *netif_api;
tai_host_interface_api_t *hostif_api;

int fd;
std::queue<std::pair<bool, std::string>> q;
std::mutex m;

class module {
    public:
        module(tai_object_id_t id) : m_id(id) {
            std::vector<tai_attribute_t> list;
            tai_attribute_t attr;
            attr.id = TAI_MODULE_ATTR_NUM_HOST_INTERFACES;
            list.push_back(attr);
            attr.id = TAI_MODULE_ATTR_NUM_NETWORK_INTERFACES;
            list.push_back(attr);
            auto status = module_api->get_module_attributes(id, list.size(), list.data());
            if ( status != TAI_STATUS_SUCCESS ) {
                throw std::runtime_error("faile to get attribute");
            }
            std::cout << "num hostif: " << list[0].value.u32 << std::endl;
            std::cout << "num netif: " << list[1].value.u32 << std::endl;
            create_hostif(list[0].value.u32);
            create_netif(list[1].value.u32);
        }
    private:
        tai_object_id_t m_id;
        std::vector<tai_object_id_t> netifs;
        std::vector<tai_object_id_t> hostifs;
        int create_hostif(uint32_t num);
        int create_netif(uint32_t num);
};

int module::create_netif(uint32_t num) {
    for ( int i = 0; i < num; i++ ) {
        tai_object_id_t id;
        std::vector<tai_attribute_t> list;
        tai_attribute_t attr;
        attr.id = TAI_NETWORK_INTERFACE_ATTR_INDEX;
        attr.value.u32 = i;
        list.push_back(attr);
        auto status = netif_api->create_network_interface(&id, m_id, list.size(), list.data());
        if ( status != TAI_STATUS_SUCCESS ) {
            throw std::runtime_error("failed to create network interface");
        }
        std::cout << "netif: " << id << std::endl;
        netifs.push_back(id);

        list.clear();

        attr.id = TAI_NETWORK_INTERFACE_ATTR_TX_DIS;
        attr.value.booldata = false;
        list.push_back(attr);

        attr.id = TAI_NETWORK_INTERFACE_ATTR_TX_LASER_FREQ;
        attr.value.u64 = 191300000000000;
        list.push_back(attr);

        attr.id = TAI_NETWORK_INTERFACE_ATTR_TX_GRID_SPACING;
        attr.value.u32 = TAI_NETWORK_INTERFACE_TX_GRID_SPACING_100_GHZ;
        list.push_back(attr);

        attr.id = TAI_NETWORK_INTERFACE_ATTR_MODULATION_FORMAT;
        attr.value.u32 = TAI_NETWORK_INTERFACE_MODULATION_FORMAT_DP_16_QAM;
        list.push_back(attr);

        status = netif_api->set_network_interface_attributes(id, list.size(), list.data());
        if ( status != TAI_STATUS_SUCCESS ) {
            throw std::runtime_error("failed to set netif attribute");
        }
    }
    return 0;
}

int module::create_hostif(uint32_t num) {
    for ( int i = 0; i < num; i++ ) {
        tai_object_id_t id;
        std::vector<tai_attribute_t> list;
        tai_attribute_t attr;
        attr.id = TAI_HOST_INTERFACE_ATTR_INDEX;
        attr.value.u32 = i;
        list.push_back(attr);
        auto status = hostif_api->create_host_interface(&id, m_id, list.size(), list.data());
        if ( status != TAI_STATUS_SUCCESS ) {
            throw std::runtime_error("failed to create host interface");
        }
        std::cout << "hostif: " << id << std::endl;
        hostifs.push_back(id);
    }
    return 0;
}

std::map<tai_object_id_t, module*> modules;

void module_presence(bool present, char* location) {
    uint64_t v;
    std::lock_guard<std::mutex> g(m);
    q.push(std::pair<bool, std::string>(present, std::string(location)));
    write(fd, &v, sizeof(uint64_t));
}

tai_status_t create_module(const std::string& location, tai_object_id_t& m_id) {
    std::vector<tai_attribute_t> list;
    tai_attribute_t attr;
    attr.id = TAI_MODULE_ATTR_LOCATION;
    attr.value.charlist.count = location.size();
    attr.value.charlist.list = (char*)location.c_str();
    list.push_back(attr);
    return module_api->create_module(&m_id, list.size(), list.data());
}

int main() {
    tai_log_set(tai_api_t(0), TAI_LOG_LEVEL_INFO);

    tai_service_method_table_t services;

    fd = eventfd(0, 0);

    services.module_presence = module_presence;

    auto status = tai_api_initialize(0, &services);
    if ( status != TAI_STATUS_SUCCESS ) {
        std::cout << "failed to init" << std::endl;
        return 1;
    }

    status = tai_api_query(TAI_API_MODULE, (void **)(&module_api));
    if ( status != TAI_STATUS_SUCCESS ) {
        return 1;
    }

    status = tai_api_query(TAI_API_NETWORKIF, (void **)(&netif_api));
    if ( status != TAI_STATUS_SUCCESS ) {
        return 1;
    }

    status = tai_api_query(TAI_API_HOSTIF, (void **)(&hostif_api));
    if ( status != TAI_STATUS_SUCCESS ) {
        return 1;
    }

    while (true) {
        uint64_t v;
        read(fd, &v, sizeof(uint64_t));
        {
            std::lock_guard<std::mutex> g(m);
            while ( ! q.empty() ) {
                auto p = q.front();
                std::cout << "present: " << p.first << ", loc: " << p.second << std::endl;
                if ( p.first ) {
                    tai_object_id_t m_id = 0;
                    status = create_module(p.second, m_id);
                    if ( status != TAI_STATUS_SUCCESS ) {
                        std::cerr << "failed to create module: " << status << std::endl;
                        return 1;
                    }
                    std::cout << "module id: " << m_id << std::endl;
                    modules[m_id] = new module(m_id);
                }
                q.pop();
            }
        }
    }
}
