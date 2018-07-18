/**
 * @file    voyager.h
 * @brief   Voyager platform definitions
 * @author  Scott Emery <scotte@cumulusnetworks.com>
 *
 * @copyright Copyright (C) 2018 Cumulus Networks, Inc. All rights reserved
 *
 * @remark  This source code is licensed under the BSD 3-Clause license found in
 *          the LICENSE file in the root directory of this source tree.
 */

#ifndef VOYAGER_H__
#define VOYAGER_H__

/** @brief The number of AC400's in Voyager */
#define VOYAGER_NUM_AC400 2

/** @brief The number of network interfaces on an AC400 */
#define AC400_NUM_NETIF 2

/** @brief The number of host interfaces on an AC400 */
#define AC400_NUM_HOSTIF 4

/** @brief The number of host lanes per host interface */
#define AC400_NUM_HOST_LANES 4

/** @brief The number of network interfaces on Voyager */
#define VOYAGER_NUM_NETIF (VOYAGER_NUM_AC400*AC400_NUM_NETIF)

/** @brief The number of host interfaces on Voyager */
#define VOYAGER_NUM_HOSTIF (VOYAGER_NUM_AC400*AC400_NUM_HOSTIF)

/** @brief The path to the CPLD's sysfs attributes */
#define CPLD_PATH "/sys/bus/platform/devices/syscpld"

/** @brief The AC400 TAI adapter uses the following format for object ids */
typedef struct _ac400_object_id_t {
    uint8_t type;
    uint8_t reserved;
    uint32_t value;
} ac400_object_id_t;

/** @brief The supported AC400 part numbers */
#define VEND_PN_EXPECT_1 "AC400-003"
#define VEND_PN_EXPECT_2 "AC400-004"

/**
 *  @brief Return the value of the global alarm signal
 *
 *  @param [in] module_id The module ID of the module being accessed
 *  @param [out] glb_alrm A pointer to a bool to place the current global alarm
 *         value.
 *
 *  @return 0 on sucess, < 0 Linux error code
 */
extern int ac400_get_glb_alrm(
   _In_ tai_object_id_t     module_id,
   _Out_ bool              *glb_alrm) __attribute__ ((visibility ("hidden")));

/**
 *  @brief Return the value of the RXLOS signal
 *
 *  @param [in] module_id The module ID of the module being accessed
 *  @param [in] networkif The number of the network interface, 1 or 2
 *  @param [out] rxlos A pointer to a bool to place the current rxlos value.
 *
 *  @return 0 on sucess, < 0 Linux error code
 */
extern int ac400_get_rxlos(
   _In_ tai_object_id_t     module_id,
   _In_ int                 networkif,
   _Out_ bool              *rxlos) __attribute__ ((visibility ("hidden")));

/**
 *  @brief Return the value of the MOD_ABS ignal
 *
 *  @param [in] module_id The module ID of the module being accessed
 *  @param [out] mod_abs A pointer to a bool to place the current mod_abs value.
 *
 *  @return 0 on sucess, < 0 Linux error code
 */
extern int ac400_get_mod_abs(
   _In_ tai_object_id_t     module_id,
   _Out_ bool              *mod_abs) __attribute__ ((visibility ("hidden")));

/**
 *  @brief Return the value of the TXDIS signal
 *
 *  @param [in] module_id The module ID of the module being accessed
 *  @param [in] networkif The number of the network interface, 1 or 2
 *  @param [out] txdis A pointer to a bool to place the current txdis value.
 *
 *  @return 0 on sucess, < 0 Linux error code
 */
extern int ac400_get_txdis(
   _In_ tai_object_id_t     module_id,
   _In_ int                 networkif,
   _Out_ bool              *txdis) __attribute__ ((visibility ("hidden")));

/**
 *  @brief Set the value of the TXDIS signal
 *
 *  @param [in] module_id The module ID of the module being accessed
 *  @param [in] networkif The number of the network interface, 1 or 2
 *  @param [in] txdis The value to set the txdis signal
 *
 *  @return 0 on sucess, < 0 Linux error code
 */
extern int ac400_set_txdis(
   _In_ tai_object_id_t     module_id,
   _In_ int                 networkif,
   _In_ bool                txdis) __attribute__ ((visibility ("hidden")));

/**
 *  @brief Return the value of the MOD_LOPWR signal
 *
 *  @param [in] module_id The module ID of the module being accessed
 *  @param [out] mod_lopwr A pointer to a bool to place the current mod_lopwr
 *         value.
 *
 *  @return 0 on sucess, < 0 Linux error code
 */
extern int ac400_get_mod_lopwr(
   _In_ tai_object_id_t     module_id,
   _Out_ bool              *mod_lopwr) __attribute__ ((visibility ("hidden")));

/**
 *  @brief Set the value of the MOD_LOPWR signal
 *
 *  @param [in] module_id The module ID of the module being accessed
 *  @param [in] mod_lopwr The value to set the mod_lopwr signal
 *
 *  @return 0 on sucess, < 0 Linux error code
 */
extern int ac400_set_mod_lopwr(
   _In_ tai_object_id_t     module_id,
   _In_ bool                mod_lowwr) __attribute__ ((visibility ("hidden")));

/**
 *  @brief Return the value of the PM_SYNC signal
 *
 *  @param [in] module_id The module ID of the module being accessed
 *  @param [out] pm_sync A pointer to a bool to place the current pm_sync value.
 *
 *  @return 0 on sucess, < 0 Linux error code
 */
extern int ac400_get_pm_sync(
   _In_ tai_object_id_t     module_id,
   _Out_ bool              *pm_sync) __attribute__ ((visibility ("hidden")));

/**
 *  @brief Set the value of the PM_SYNC signal
 *
 *  @param [in] module_id The module ID of the module being accessed
 *  @param [in] pm_sync The value to set the pm_sync signal
 *
 *  @return 0 on sucess, < 0 Linux error code
 */
extern int ac400_set_pm_sync(
   _In_ tai_object_id_t     module_id,
   _In_ bool                pm_sync) __attribute__ ((visibility ("hidden")));

/**
 *  @brief Return the value of the RESET signal
 *
 *  @param [in] module_id The module ID of the module being accessed
 *  @param [out] reset A pointer to a bool to place the current reset value.
 *
 *  @return 0 on sucess, < 0 Linux error code
 */
extern int ac400_get_reset(
   _In_ tai_object_id_t     module_id,
   _Out_ bool              *reset) __attribute__ ((visibility ("hidden")));

/**
 *  @brief Set the value of the RESET signal
 *
 *  @param [in] module_id The module ID of the module being accessed
 *  @param [in] reset The value to set the reset signal
 *
 *  @return 0 on sucess, < 0 Linux error code
 */
extern int ac400_set_reset(
   _In_ tai_object_id_t     module_id,
   _In_ bool                reset) __attribute__ ((visibility ("hidden")));

/**
 *  @brief Return the value of the POWER signal
 *
 *  @param [in] module_id The module ID of the module being accessed
 *  @param [out] power A pointer to a bool to place the current power value.
 *
 *  @return 0 on sucess, < 0 Linux error code
 */
extern int ac400_get_power(
   _In_ tai_object_id_t     module_id,
   _Out_ bool              *power) __attribute__ ((visibility ("hidden")));

/**
 *  @brief Set the value of the POWER signal
 *
 *  @param [in] module_id The module ID of the module being accessed
 *  @param [in] power The value to set the power signal
 *
 *  @return 0 on sucess, < 0 Linux error code
 */
extern int ac400_set_power(
   _In_ tai_object_id_t     module_id,
   _In_ bool                power) __attribute__ ((visibility ("hidden")));

/**
 *  @brief Return the value of the MDIO device_type
 *
 *  @param [out] type A pointer to an int to place the current MDIO device_type.
 *
 *  @return 0 on sucess, < 0 Linux error code
 */
extern int ac400_get_device_type(
   _Out_ int               *type) __attribute__ ((visibility ("hidden")));

/**
 *  @brief Set the value of the MDIO device_type
 *
 *  @param [in] type The value to set the MDIO device type
 *
 *  @return 0 on sucess, < 0 Linux error code
 */
extern int ac400_set_device_type(
   _In_ int                 type) __attribute__ ((visibility ("hidden")));

/**
 *  @brief Perform an MDIO read from the AC400 module
 *
 *  @param [in] module_id The module ID of the module being accessed
 *  @param [in] reg The MDIO address of the register to read
 *  @param [out] value A pointer to a u16 to place the value read from the AC400
 *
 *  @return 0 on sucess, < 0 Linux error code
 */
extern int ac400_mdio_read(
   _In_ tai_object_id_t     module_id,
   _In_ uint16_t            reg,
   _Out_ uint16_t          *value) __attribute__ ((visibility ("hidden")));

/**
 *  @brief Perform an MDIO write to the AC400 module
 *
 *  @param [in] module_id The module ID of the module being accessed
 *  @param [in] reg The MDIO address of the register to write
 *  @param [out] value A u16 value to write to the AC400
 *
 *  @return 0 on sucess, < 0 Linux error code
 */
extern int ac400_mdio_write(
   _In_ tai_object_id_t     module_id,
   _In_ uint16_t            reg,
   _In_ uint16_t            value) __attribute__ ((visibility ("hidden")));

/**
 * @brief Validate the module location and initialize the module_id handle.
 *
 * @param [in] mod_addr A pointer to an s8list (string) which gives the module
 *        index, either "1" or "2".
 * @param [out] module_id A pointer to the object id for the module object.
 *
 * @return 0 on success, or -1 on failure.
 */
extern int ac400_set_module_id(_In_ const tai_attribute_value_t * mod_addr,
                               _Out_ tai_object_id_t            * module_id)
                               __attribute__ ((visibility ("hidden")));

/**
 * @brief Given a module_id, return the module location.
 *
 * @param [in] module_id A pointer to a module object id.
 * @param [out] location A pointer to location where the module location will be
 *        stored.
 */
extern void ac400_get_module_location(_In_ tai_object_id_t module_id,
                                      _Out_ char         * location)
                                      __attribute__ ((visibility ("hidden")));

#endif  /* VOYAGER_H__ */
