#include "sff_fsm.hpp"

namespace tai::sff {

    static const std::string to_string(FSMState s) {
        switch (s) {
        case FSM_STATE_INIT:
            return "init";
        case FSM_STATE_WAITING_CONFIGURATION:
            return "waiting-configuration";
        case FSM_STATE_READY:
            return "ready";
        case FSM_STATE_END:
            return "end";
        }
        return "unknown";
    }

    // trim from start (in place)
    static inline void ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
            return !std::isspace(ch);
        }));
    }

    // trim from end (in place)
    static inline void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }

    // trim from both ends (in place)
    static inline void trim(std::string &s) {
        ltrim(s);
        rtrim(s);
    }

    FSM::FSM(Location loc, const tai_service_method_table_t* services) : m_loc(loc), m_services(services), m_module(nullptr), m_netif{}, m_hostif{}, m_no_transit(false) {
        m_eeprom = std::ifstream(loc + "/eeprom");
        if ( !m_eeprom ) {
            TAI_ERROR("failed to open eeprom");
            throw Exception(TAI_STATUS_ITEM_NOT_FOUND);
        }
    }

    bool FSM::configured() {
        return m_module != nullptr;
    }

    // this can be called only once during the lifecycle of this FSM
    // remove is not considered yet
    // returns 0 on success. otherwise -1
    int FSM::set_module(S_Module module) {
        if ( m_module != nullptr || module == nullptr ) {
            return -1;
        }
        m_module = module;
        return 0;
    }

    // returns 0 on success. otherwise -1
    // remove is not considered yet
    int FSM::set_netif(S_NetIf netif, int index) {
        if ( index < 0 || index >= SFF_NUM_NETIF ) {
            return -1;
        }
        if ( m_netif[index] != nullptr || netif == nullptr ) {
            return -1;
        }
        m_netif[index] = netif;
        return 0;
    }

    // returns 0 on success. otherwise -1
    // remove is not considered yet
    int FSM::set_hostif(S_HostIf hostif, int index) {
        if ( index < 0 || index >= SFF_NUM_HOSTIF ) {
            return -1;
        }
        if ( m_hostif[index] != nullptr || hostif == nullptr ) {
            return -1;
        }
        m_hostif[index] = hostif;
        return 0;
    }

    tai_status_t FSM::remove_module() {
        if ( m_module == nullptr ) {
            return TAI_STATUS_ITEM_NOT_FOUND;
        }
        for ( int i = 0; i < SFF_NUM_NETIF; i++ ) {
            if ( m_netif[i] != nullptr ) {
                TAI_WARN("can't remove a module before removing its sibling netifs");
                return TAI_STATUS_OBJECT_IN_USE;
            }
        }
        for ( int i = 0; i < SFF_NUM_HOSTIF; i++ ) {
            if ( m_hostif[i] != nullptr ) {
                TAI_WARN("can't remove a module before removing its sibling hostifs");
                return TAI_STATUS_OBJECT_IN_USE;
            }
        }
        transite(FSM_STATE_END);
        while(true) {
            auto s = get_state();
            if ( s == FSM_STATE_END ) {
                break;
            }
            usleep(100000);
        }
        m_module = nullptr;
        return TAI_STATUS_SUCCESS;
    }

    tai_status_t FSM::remove_netif(int index) {
        if ( index < 0 || index >= SFF_NUM_NETIF || m_netif[index] == nullptr ) {
            return TAI_STATUS_ITEM_NOT_FOUND;
        }
        m_netif[index] = nullptr;
        return TAI_STATUS_SUCCESS;
    }

    tai_status_t FSM::remove_hostif(int index) {
        if ( index < 0 || index >= SFF_NUM_HOSTIF || m_hostif[index] == nullptr ) {
            return TAI_STATUS_ITEM_NOT_FOUND;
        }
        m_hostif[index] = nullptr;
        return TAI_STATUS_SUCCESS;
    }

    fsm_state_change_callback FSM::state_change_cb() {
        return std::bind(&FSM::_state_change_cb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    }

    FSMState FSM::_state_change_cb(FSMState current, FSMState next, void* user) {
        if ( m_module != nullptr ) {
            tai_attribute_t oper;
            oper.id = TAI_MODULE_ATTR_OPER_STATUS;
            if ( next == FSM_STATE_READY ) {
                oper.value.s32 = TAI_MODULE_OPER_STATUS_READY;
            } else {
                oper.value.s32 = TAI_MODULE_OPER_STATUS_INITIALIZE;
            }
            auto& config = m_module->config();
            config.set_readonly(oper);
            m_module->notify(TAI_MODULE_ATTR_NOTIFY, {
                    TAI_MODULE_ATTR_OPER_STATUS,
            });
        }
        TAI_INFO("%s -> %s", to_string(current).c_str(), to_string(next).c_str());
        return next;
    }

    bool FSM::is_present() {
        char buf;
        m_eeprom.read(&buf, 1);
        return m_eeprom.good();
    }

    fsm_callback FSM::cb(FSMState state) {
        switch (state) {
        case FSM_STATE_INIT:
            return std::bind(&FSM::_init_cb, this, std::placeholders::_1, std::placeholders::_2);
        case FSM_STATE_WAITING_CONFIGURATION:
            return std::bind(&FSM::_waiting_configuration_cb, this, std::placeholders::_1, std::placeholders::_2);
        case FSM_STATE_READY:
            return std::bind(&FSM::_ready_cb, this, std::placeholders::_1, std::placeholders::_2);
        }
        return nullptr;
    }

    // wait eeprom get readable
    FSMState FSM::_init_cb(FSMState current, void* user) {
        fd_set fs;
        auto evfd = get_event_fd();
        itimerspec interval = {{1,0}, {0,1}};
        auto tfd = timerfd_create(CLOCK_REALTIME, 0);
        timerfd_settime(tfd, 0, &interval, NULL);
        FSMState next;

        bool first = true, prev;

        while (true) {
            FD_ZERO(&fs);
            FD_SET(tfd, &fs);
            FD_SET(evfd, &fs);
            select(FD_SETSIZE, &fs, NULL, NULL, NULL);
            if (FD_ISSET(evfd, &fs)) {
                uint64_t r;
                read(evfd, &r, sizeof(uint64_t));
                next = next_state();
                if ( next == FSM_STATE_END ) {
                    goto ret;
                }
            }
            if (FD_ISSET(tfd, &fs)) {
                uint64_t r;
                read(tfd, &r, sizeof(uint64_t));
                auto present = is_present();
                if ( first || (present != prev) ) {
                    first = false;
                    if ( m_services != nullptr && m_services->module_presence != nullptr ) {
                        m_services->module_presence(present, const_cast<char*>(m_loc.c_str()));
                    }
                }
                prev = present;
                if ( present ) {
                    next = FSM_STATE_WAITING_CONFIGURATION;
                    goto ret;
                }
            }
        }
    ret:
        close(tfd);
        return next;
    }

    // wait module get created ( check by configured() )
    FSMState FSM::_waiting_configuration_cb(FSMState current, void* user) {

        fd_set fs;
        auto evfd = get_event_fd();
        itimerspec interval = {{1,0}, {0,1}};
        auto tfd = timerfd_create(CLOCK_REALTIME, 0);
        timerfd_settime(tfd, 0, &interval, NULL);
        FSMState next;

        while (true) {
            FD_ZERO(&fs);
            FD_SET(evfd, &fs);
            FD_SET(tfd, &fs);
            select(FD_SETSIZE, &fs, NULL, NULL, NULL);
            if (FD_ISSET(evfd, &fs)) {
                uint64_t r;
                read(evfd, &r, sizeof(uint64_t));
                next = next_state();
                goto ret;
            }
            if (FD_ISSET(tfd, &fs)) {
                uint64_t r;
                read(tfd, &r, sizeof(uint64_t));
                if ( configured() && !m_no_transit ) {
                    return FSM_STATE_READY;
                }
            }
        }

    ret:
        close(tfd);
        return next;
    }

    FSMState FSM::_ready_cb(FSMState current, void* user) {
        fd_set fs;
        auto evfd = get_event_fd();
        itimerspec interval = {{10,0}, {0,1}}; // 10s PM interval
        auto tfd = timerfd_create(CLOCK_REALTIME, 0);
        timerfd_settime(tfd, 0, &interval, NULL);
        FSMState next;

        while (true) {
            FD_ZERO(&fs);
            FD_SET(evfd, &fs);
            FD_SET(tfd, &fs);
            select(FD_SETSIZE, &fs, NULL, NULL, NULL);
            if (FD_ISSET(evfd, &fs)) {
                uint64_t r;
                read(evfd, &r, sizeof(uint64_t));
                next = next_state();
                goto ret;
            }
            if (FD_ISSET(tfd, &fs)) {
                uint64_t r;
                read(tfd, &r, sizeof(uint64_t));

                for ( int i = 0; i < SFF_NUM_NETIF; i++ ) {
                    if ( m_netif[i] != nullptr ) {
                        m_netif[i]->notify(TAI_NETWORK_INTERFACE_ATTR_NOTIFY, {
                            TAI_NETWORK_INTERFACE_ATTR_CURRENT_INPUT_POWER,
                            TAI_NETWORK_INTERFACE_ATTR_CURRENT_OUTPUT_POWER,
                        });
                    }
                }

                if ( m_module != nullptr ) {
                    m_module->notify(TAI_MODULE_ATTR_NOTIFY, {
                        TAI_MODULE_ATTR_TEMP,
                        TAI_MODULE_ATTR_POWER,
                    });
                }

            }
        }

    ret:
        close(tfd);
        return next;
    }

    tai_status_t FSM::eeprom_get_str(int address, int size, tai_attribute_t* const attr) {
        m_eeprom.seekg(address);
        auto p = std::make_unique<char[]>(size);
        auto buf = p.get();
        m_eeprom.read(buf, size);
        std::string s(buf, size);
        trim(s);
        auto v = attr->value.charlist.count;
        attr->value.charlist.count = s.size() + 1;
        if ( v < (s.size() + 1)) {
            return TAI_STATUS_BUFFER_OVERFLOW;
        }
        std::strncpy(attr->value.charlist.list, s.c_str(), v);
        return TAI_STATUS_SUCCESS;
    }

    tai_status_t FSM::eeprom_get_temp(int address, int size, tai_attribute_t* const attr) {
        m_eeprom.seekg(address);
        auto p = std::make_unique<char[]>(size);
        auto buf = p.get();
        m_eeprom.read(buf, size);
        auto temp = static_cast<int>(buf[0] * 256  + buf[1]);
        if ( temp > 0x7FFF ) {
            temp -= 65536;
        }
        attr->value.flt = static_cast<float>(temp)/256;
        return TAI_STATUS_SUCCESS;
    }

    tai_status_t FSM::eeprom_get_voltage(int address, int size, tai_attribute_t* const attr) {
        m_eeprom.seekg(address);
        auto p = std::make_unique<char[]>(size);
        auto buf = p.get();
        m_eeprom.read(buf, size);
        auto temp = static_cast<int>(buf[0] * 256  + buf[1]);
        attr->value.flt = static_cast<float>(temp)/10000;
        return TAI_STATUS_SUCCESS;
    }

    tai_status_t FSM::eeprom_get_power_dbm(int address, int size, tai_attribute_t* const attr) {
        m_eeprom.seekg(address);
        auto p = std::make_unique<char[]>(size);
        auto buf = p.get();
        m_eeprom.read(buf, size);
        auto tmp = static_cast<float>(static_cast<int>(buf[0] * 256  + buf[1]))/10000;
        if ( tmp < 0.001 ) {
            attr->value.flt = -30; // by convention, -30dBm is the lowest legal value (OOM)
        } else {
            attr->value.flt = 10 * std::log10(tmp);
        }
        return TAI_STATUS_SUCCESS;
    }

    tai_status_t FSM::get(tai_object_type_t type, tai_object_id_t oid, tai_attribute_t* const attr) {
        switch (type) {
        case TAI_OBJECT_TYPE_MODULE:
            switch (attr->id) {
                case TAI_MODULE_ATTR_VENDOR_NAME:
                    return eeprom_get_str(148, 16, attr);
                case TAI_MODULE_ATTR_VENDOR_PART_NUMBER:
                    return eeprom_get_str(168, 16, attr);
                case TAI_MODULE_ATTR_VENDOR_SERIAL_NUMBER:
                    return eeprom_get_str(196, 16, attr);
                case TAI_MODULE_ATTR_TEMP:
                    return eeprom_get_temp(22, 2, attr);
                case TAI_MODULE_ATTR_POWER:
                    return eeprom_get_voltage(26, 2, attr);
                default:
                    return TAI_STATUS_NOT_SUPPORTED;
            }
        case TAI_OBJECT_TYPE_NETWORKIF:
            {
                auto index = static_cast<int>(oid & 0xff);
                switch (attr->id) {
                    case TAI_NETWORK_INTERFACE_ATTR_CURRENT_INPUT_POWER:
                        return eeprom_get_power_dbm(34 + index*2, 2, attr);
                    case TAI_NETWORK_INTERFACE_ATTR_CURRENT_OUTPUT_POWER:
                        return eeprom_get_power_dbm(50 + index*2, 2, attr);
                    default:
                        return TAI_STATUS_NOT_SUPPORTED;
                }
            }
        default:
            return TAI_STATUS_NOT_SUPPORTED;
        }
        return TAI_STATUS_SUCCESS;
    }

    tai_status_t FSM::set(tai_object_type_t type, tai_object_id_t oid, const tai_attribute_t* const attribute, FSMState* state) {
        return TAI_STATUS_NOT_SUPPORTED;
    }

};
