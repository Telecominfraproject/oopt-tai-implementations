#ifndef __SFF_FSM_HPP__
#define __SFF_FSM_HPP__

#include "fsm.hpp"

#include <fstream>
#include <cstdio>
#include <atomic>

namespace tai::sff {

    using namespace tai::framework;

    // Total number of modules
    const uint8_t SFF_NUM_MODULE = 4;
    // The number of network interface which one module has
    const uint8_t SFF_NUM_NETIF = 4;
    // The number of host interface which one module has
    const uint8_t SFF_NUM_HOSTIF = 1;

    class Module;
    class NetIf;
    class HostIf;

    using S_Module = std::shared_ptr<Module>;
    using S_NetIf  = std::shared_ptr<NetIf>;
    using S_HostIf = std::shared_ptr<HostIf>;

    class FSM : public tai::framework::FSM {
        // requirements to inherit tai::FSM
        public:
            bool configured();
        private:
            fsm_state_change_callback state_change_cb();
            fsm_callback cb(FSMState state);

        public:
            FSM(Location loc, const tai_service_method_table_t* services);

            int set_module(S_Module module);
            int set_netif(S_NetIf   netif, int index);
            int set_hostif(S_HostIf hostif, int index);

            tai_status_t remove_module();
            tai_status_t remove_netif(int index);
            tai_status_t remove_hostif(int index);

            Location location() {
                return m_loc;
            }

            bool is_present();

            tai_status_t get(tai_object_type_t type, tai_object_id_t oid, tai_attribute_t* const attribute);
            tai_status_t set(tai_object_type_t type, tai_object_id_t oid, const tai_attribute_t* const attribute, FSMState* state);

        private:
            FSMState _state_change_cb(FSMState current, FSMState next, void* user);

            FSMState _init_cb(FSMState current, void* user);
            FSMState _waiting_configuration_cb(FSMState current, void* user);
            FSMState _ready_cb(FSMState current, void* user);

            S_Module m_module;
            S_NetIf m_netif[SFF_NUM_NETIF];
            S_HostIf m_hostif[SFF_NUM_HOSTIF];

            std::atomic<bool> m_no_transit;

            const tai_service_method_table_t* m_services;
            const Location m_loc;

            tai_status_t eeprom_get_str(int address, int size, tai_attribute_t* const attr);
            tai_status_t eeprom_get_temp(int address, int size, tai_attribute_t* const attr);
            tai_status_t eeprom_get_voltage(int address, int size, tai_attribute_t* const attr);
            tai_status_t eeprom_get_power_dbm(int address, int size, tai_attribute_t* const attr);

            std::ifstream m_eeprom;
    };

    using S_FSM = std::shared_ptr<FSM>;

};

#endif
