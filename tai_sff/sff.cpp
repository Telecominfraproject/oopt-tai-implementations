#include "sff.hpp"
#include "logger.hpp"

#include <glob.h>

namespace tai::sff {

    static const std::string SYSFS_I2C_DIR = "/sys/bus/i2c/devices";

    Platform::Platform(const tai_service_method_table_t * services) : tai::framework::Platform(services) {

        if ( services == nullptr || services->module_presence == nullptr ) {
            return;
        }

        glob_t pglob;
        auto ret = glob((SYSFS_I2C_DIR + "/*-0050").c_str(), 0, nullptr, &pglob);
        if ( ret != 0 ) {
            globfree(&pglob);
            TAI_ERROR("glob failed");
            throw Exception(TAI_STATUS_FAILURE);
        }
        for ( int i = 0; i < pglob.gl_pathc; i++ ) {
            auto loc = std::string(pglob.gl_pathv[i]);
            auto eeprom = std::ifstream(loc + "/eeprom");
            if ( !eeprom ) {
                continue;
            }
            auto fsm = std::make_shared<sff::FSM>(loc, services);
            if ( fsm->start() < 0 ) {
                TAI_ERROR("failed to start FSM for module %s", loc.c_str());
                throw Exception(TAI_STATUS_FAILURE);
            }
            m_fsms[loc] = fsm;
        }
        globfree(&pglob);
    }

    tai_status_t Platform::create(tai_object_type_t type, tai_object_id_t module_id, uint32_t count, const tai_attribute_t *list, tai_object_id_t *id) {
        std::shared_ptr<tai::framework::BaseObject> obj;
        try {
            switch (type) {
            case TAI_OBJECT_TYPE_MODULE:
                {
                    tai::framework::Location loc;
                    for ( auto i = 0; i < count; i++ ) {
                        if ( list[i].id == TAI_MODULE_ATTR_LOCATION ) {
                            loc = tai::framework::Location(list[i].value.charlist.list, list[i].value.charlist.count);
                            break;
                        }
                    }
                    if ( loc == "" ) {
                        return TAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
                    }

                    S_FSM fsm;

                    if ( m_services != nullptr && m_services->module_presence != nullptr ) {
                        auto it = m_fsms.find(loc);
                        if ( it == m_fsms.end() ) {
                            return TAI_STATUS_INVALID_PARAMETER;
                        }
                        fsm = std::dynamic_pointer_cast<sff::FSM>(m_fsms[loc]);
                        if ( !fsm->is_present() ) {
                            TAI_ERROR("module is not present: %s", loc.c_str());
                            return TAI_STATUS_FAILURE;
                        }
                    } else {
                        auto it = m_fsms.find(loc);
                        if ( it != m_fsms.end() ) {
                            TAI_ERROR("FSM already exists for module: %s", loc.c_str());
                            return TAI_STATUS_ITEM_ALREADY_EXISTS;
                        }
                        fsm = std::make_shared<sff::FSM>(loc, m_services);
                        m_fsms[loc] = fsm;

                        if ( fsm->start() < 0 ) {
                            TAI_ERROR("failed to start FSM for module %s", loc.c_str());
                            return TAI_STATUS_FAILURE;
                        }
                    }

                    auto m = std::make_shared<Module>(count, list, fsm);
                    if ( fsm->set_module(m) < 0 ) {
                        TAI_ERROR("failed to set module to FSM for module %s", loc.c_str());
                        return TAI_STATUS_FAILURE;
                    }
                    obj = m;
                }
                break;
            case TAI_OBJECT_TYPE_NETWORKIF:
            case TAI_OBJECT_TYPE_HOSTIF:
                {
                    auto t = static_cast<tai_object_type_t>(module_id >> OBJECT_TYPE_SHIFT);
                    if ( t != TAI_OBJECT_TYPE_MODULE ) {
                        return TAI_STATUS_INVALID_OBJECT_ID;
                    }
                    auto it = m_objects.find(module_id);
                    if ( it == m_objects.end() ) {
                        return TAI_STATUS_UNINITIALIZED;
                    }
                    auto module = std::dynamic_pointer_cast<Module>(it->second);
                    if ( type == TAI_OBJECT_TYPE_NETWORKIF ) {
                        auto netif = std::make_shared<NetIf>(module, count, list);
                        module->fsm()->set_netif(netif, (netif->id() & 0xff));
                        obj = netif;
                    } else {
                        auto hostif = std::make_shared<HostIf>(module, count, list);
                        module->fsm()->set_hostif(hostif, (hostif->id() & 0xff));
                        obj = hostif;
                    }
                }
                break;
            default:
                return TAI_STATUS_NOT_SUPPORTED;
            }
        } catch (tai_status_t e) {
            return e;
        } catch (...) {
            return TAI_STATUS_FAILURE;
        }

        auto oid = obj->id();
        auto it = m_objects.find(oid);
        if ( it != m_objects.end() ) {
            return TAI_STATUS_ITEM_ALREADY_EXISTS;
        }
        m_objects[oid] = obj;
        *id = oid;
        return TAI_STATUS_SUCCESS;
    }

    tai_object_type_t Platform::get_object_type(tai_object_id_t id) {
        auto it = m_objects.find(id);
        if ( it == m_objects.end() ) {
            return TAI_OBJECT_TYPE_NULL;
        }
        auto type = static_cast<tai_object_type_t>(id >> OBJECT_TYPE_SHIFT);
        switch (type) {
        case TAI_OBJECT_TYPE_MODULE:
        case TAI_OBJECT_TYPE_NETWORKIF:
        case TAI_OBJECT_TYPE_HOSTIF:
            return type;
        }
        return TAI_OBJECT_TYPE_NULL;
    }

    tai_object_id_t Platform::get_module_id(tai_object_id_t id) {
        auto it = m_objects.find(id);
        if ( it == m_objects.end() ) {
            return TAI_NULL_OBJECT_ID;
        }
        auto type = static_cast<tai_object_type_t>(id >> OBJECT_TYPE_SHIFT);
        switch (type) {
        case TAI_OBJECT_TYPE_MODULE:
            return id;
        case TAI_OBJECT_TYPE_NETWORKIF:
        case TAI_OBJECT_TYPE_HOSTIF:
            {
                auto idx = ((id >> 8) & 0xff);
                auto module_id = static_cast<tai_object_id_t>(uint64_t(TAI_OBJECT_TYPE_MODULE) << OBJECT_TYPE_SHIFT | idx);
                auto it = m_objects.find(module_id);
                if ( it == m_objects.end() ) {
                    return TAI_NULL_OBJECT_ID;
                }
                return module_id;
            }
        }
        return TAI_OBJECT_TYPE_NULL;
    }

    tai_status_t attribute_getter(tai_attribute_t* const attribute, void* user) {
        auto ctx = reinterpret_cast<context*>(user);
        return ctx->fsm->get(ctx->type, ctx->oid, attribute);
    }

    tai_status_t attribute_setter(const tai_attribute_t* const attribute, FSMState* state, void* user) {
        auto ctx = reinterpret_cast<context*>(user);
        return ctx->fsm->set(ctx->type, ctx->oid, attribute, state);
    }

    static const tai_attribute_value_t default_tai_module_num_network_interfaces = {
        .u32 = SFF_NUM_NETIF,
    };

    static const tai_attribute_value_t default_tai_module_num_host_interfaces = {
        .u32 = SFF_NUM_HOSTIF,
    };

    using M = AttributeInfo<TAI_OBJECT_TYPE_MODULE>;
    using N = AttributeInfo<TAI_OBJECT_TYPE_NETWORKIF>;
    using H = AttributeInfo<TAI_OBJECT_TYPE_HOSTIF>;

    // sadly 'auto' can't be used here
    template <> const AttributeInfoMap<TAI_OBJECT_TYPE_MODULE> Config<TAI_OBJECT_TYPE_MODULE>::m_info {
        sff::M(TAI_MODULE_ATTR_LOCATION),
        sff::M(TAI_MODULE_ATTR_VENDOR_NAME)
            .set_getter(&sff::attribute_getter),
        sff::M(TAI_MODULE_ATTR_VENDOR_PART_NUMBER)
            .set_getter(&sff::attribute_getter),
        sff::M(TAI_MODULE_ATTR_VENDOR_SERIAL_NUMBER)
            .set_getter(&sff::attribute_getter),
        sff::M(TAI_MODULE_ATTR_NUM_NETWORK_INTERFACES)
            .set_default(&tai::sff::default_tai_module_num_network_interfaces),
        sff::M(TAI_MODULE_ATTR_NUM_HOST_INTERFACES)
            .set_default(&tai::sff::default_tai_module_num_host_interfaces),
        sff::M(TAI_MODULE_ATTR_OPER_STATUS),
        sff::M(TAI_MODULE_ATTR_TEMP)
            .set_getter(&sff::attribute_getter),
        sff::M(TAI_MODULE_ATTR_POWER)
            .set_getter(&sff::attribute_getter),
        sff::M(TAI_MODULE_ATTR_NOTIFY),
    };

    template <> const AttributeInfoMap<TAI_OBJECT_TYPE_NETWORKIF> Config<TAI_OBJECT_TYPE_NETWORKIF>::m_info {
        sff::N(TAI_NETWORK_INTERFACE_ATTR_INDEX),
        sff::N(TAI_NETWORK_INTERFACE_ATTR_CURRENT_OUTPUT_POWER)
            .set_getter(&sff::attribute_getter),
        sff::N(TAI_NETWORK_INTERFACE_ATTR_CURRENT_INPUT_POWER)
            .set_getter(&sff::attribute_getter),
        sff::N(TAI_NETWORK_INTERFACE_ATTR_NOTIFY),
    };

    template <> const AttributeInfoMap<TAI_OBJECT_TYPE_HOSTIF> Config<TAI_OBJECT_TYPE_HOSTIF>::m_info {
        sff::H(TAI_HOST_INTERFACE_ATTR_INDEX),
    };
}
