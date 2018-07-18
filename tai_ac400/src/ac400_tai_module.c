/**
 *  @file    ac400_tai_module.c
 *  @brief   The TAI module interface routines
 *  @author  Scott Emery <scotte@cumulusnetworks.com>
 *
 *  @copyright Copyright (C) 2017 Cumulus Networks, Inc. All rights reserved
 *
 *  @remark  This source code is licensed under the BSD 3-Clause license found
 *           in the LICENSE file in the root directory of this source tree.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "voyager_tai_adapter.h"

#undef  __TAI_MODULE__
#define __TAI_MODULE__ TAI_API_MODULE

static tai_module_notification_t notification_callbacks;
static bool                      module_init[VOYAGER_NUM_AC400];

/**
 * @brief Perform initialization of an AC400 module.
 *
 * @param [in] module_id The module which is being initialized
 *
 * @return tai_status_t #TAI_STATUS_SUCCESS on success or an error code on
 *         failure.
 */
static tai_status_t ac400_module_init(_In_ tai_object_id_t module_id)
{
    int ret;
    int reps;
    bool glb_alrm;

    /* Power on the AC400 and assert reset, low power, and txdis */
    ret = ac400_set_reset(module_id, true);
    if (ret) {
        TAI_SYSLOG_ERROR("Unable to place module in reset");
        return TAI_STATUS_FAILURE;
    }

    ret = ac400_set_power(module_id, true);
    if (ret) {
        TAI_SYSLOG_ERROR("Unable to turn module power on");
        return TAI_STATUS_FAILURE;
    }

    ret = ac400_set_mod_lopwr(module_id, true);
    if (ret) {
        TAI_SYSLOG_ERROR("Unable to assert the module low power signal");
        return TAI_STATUS_FAILURE;
    }

    ret = ac400_set_txdis(module_id, 1, true);
    if (ret) {
        TAI_SYSLOG_ERROR("Unable to disable transmit power for interface 1");
        return TAI_STATUS_FAILURE;
    }

    ret = ac400_set_txdis(module_id, 2, true);
    if (ret) {
        TAI_SYSLOG_ERROR("Unable to disable transmit power for interface 2");
        return TAI_STATUS_FAILURE;
    }

    /* Take the module out of reset */
    ret = ac400_set_reset(module_id, false);
    if (ret) {
        TAI_SYSLOG_ERROR("Unable to place module in reset");
        return TAI_STATUS_FAILURE;
    }

    /* Wait until global alarm is asserted (reset complete) */
    reps = 100;
    glb_alrm = false;
    do {
        ac400_get_glb_alrm(module_id, &glb_alrm);
        if (glb_alrm)
            break;
        usleep(200000);
    }
    while (--reps);
    if (!reps) {
        TAI_SYSLOG_ERROR("Timed out waiting for module to complete reset");
        return TAI_STATUS_FAILURE;
    }

    ret = ac400_set_device_type(1);
    if (ret) {
        TAI_SYSLOG_ERROR("Unable to set the MDIO device type");
        return TAI_STATUS_FAILURE;
    }

    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Retrieve the firmware versions from the AC400
 *
 * The firmware version attribute is a 2 element list of floating point values.
 *
 * @param [in] module_id The AC400 module identifier
 * @param [in,out] attr The attribute into which the firmware version are placed
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_fw_versions(
   _In_ tai_object_id_t     module_id,
   _Inout_ tai_attribute_t *attr)
{
    uint16_t word1, word2;

    /* Is there enough room in the attribute array? */
    if (attr->value.floatlist.count < 2) {
        attr->value.floatlist.count = 2;
        return TAI_STATUS_BUFFER_OVERFLOW;
    }
    attr->value.floatlist.count = 2;

    /* Get Firmware A version */
    if (ac400_mdio_read(module_id, AC400_FIRM_A_VER_NUM_X_REG, &word1)) {
        return TAI_STATUS_FAILURE;
    }
    if (ac400_mdio_read(module_id, AC400_FIRM_A_VER_NUM_Y_REG, &word2)) {
        return TAI_STATUS_FAILURE;
    }
    if (word2 >= 100) {
        attr->value.floatlist.list[0] = (float)word1 + (float)word2 / 1000;
    }
    else if (word2 >= 10) {
        attr->value.floatlist.list[0] = (float)word1 + (float)word2 / 100;
    }
    else {
        attr->value.floatlist.list[0] = (float)word1 + (float)word2 / 10;
    }

    /* Get Firmware B version */
    if (ac400_mdio_read(module_id, AC400_FIRM_B_VER_NUM_X_REG, &word1)) {
        return TAI_STATUS_FAILURE;
    }
    if (ac400_mdio_read(module_id, AC400_FIRM_B_VER_NUM_Y_REG, &word2)) {
        return TAI_STATUS_FAILURE;
    }
    if (word2 >= 100) {
        attr->value.floatlist.list[1] = (float)word1 + (float)word2 / 1000;
    }
    else if (word2 >= 10) {
        attr->value.floatlist.list[1] = (float)word1 + (float)word2 / 100;
    }
    else {
        attr->value.floatlist.list[1] = (float)word1 + (float)word2 / 10;
    }

    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Retrieve the operational status of the module
 *
 * @param [in] module_id The AC400 module identifier
 * @param [in,out] attr The attribute into which the operational status (enum)
 *        is placed
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_tai_oper_status(
   _In_ tai_object_id_t     module_id,
   _Inout_ tai_attribute_t *attr)
{
    ac400_object_id_t *ac400_obj = (ac400_object_id_t *)module_id;

    attr->value.u32 = TAI_MODULE_OPER_STATUS_INITIALIZE;
    if ( module_init[ac400_obj->value - 1] ) {
        attr->value.u32 = TAI_MODULE_OPER_STATUS_READY;
    }
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Retrieve the operational status of a module
 *
 * @param [in] module_id The module identifier
 * @param [in,out] attr The attribute into which the operational status (enum)
 *        is placed
 *
 * @return tai_status_t
 */
tai_status_t ac400_get_module_oper_status(
   _In_ tai_object_id_t     module_id,
   _Inout_ tai_attribute_t *attr)
{
    uint16_t word;
    bool reset;

    if (ac400_get_reset(module_id, &reset)) {
        return TAI_STATUS_FAILURE;
    }
    if (reset) {
        attr->value.u32 = TAI_NETWORK_INTERFACE_OPER_STATUS_RESET;
        return TAI_STATUS_SUCCESS;
    }

    if (ac400_mdio_read(module_id, AC400_MOD_STATE_REG, &word)) {
        return TAI_STATUS_FAILURE;
    }

    if (AC400_GET_BIT(word, AC400_MOD_STATE_HI_PWR_DWN))
        attr->value.u32 = TAI_NETWORK_INTERFACE_OPER_STATUS_HIGH_POWER_DOWN;
    else if (AC400_GET_BIT(word, AC400_MOD_STATE_TX_TURN_OFF))
        attr->value.u32 = TAI_NETWORK_INTERFACE_OPER_STATUS_TX_TURN_OFF;
    else if (AC400_GET_BIT(word, AC400_MOD_STATE_FAULT))
        attr->value.u32 = TAI_NETWORK_INTERFACE_OPER_STATUS_FAULT;
    else if (AC400_GET_BIT(word, AC400_MOD_STATE_READY))
        attr->value.u32 = TAI_NETWORK_INTERFACE_OPER_STATUS_READY;
    else if (AC400_GET_BIT(word, AC400_MOD_STATE_TX_TURN_ON))
        attr->value.u32 = TAI_NETWORK_INTERFACE_OPER_STATUS_TX_TURN_ON;
    else if (AC400_GET_BIT(word, AC400_MOD_STATE_TX_OFF))
        attr->value.u32 = TAI_NETWORK_INTERFACE_OPER_STATUS_TX_OFF;
    else if (AC400_GET_BIT(word, AC400_MOD_STATE_HI_PWR_UP))
        attr->value.u32 = TAI_NETWORK_INTERFACE_OPER_STATUS_HIGH_POWER_UP;
    else if (AC400_GET_BIT(word, AC400_MOD_STATE_LOW_PWR))
        attr->value.u32 = TAI_NETWORK_INTERFACE_OPER_STATUS_LOW_POWER;
    else if (AC400_GET_BIT(word, AC400_MOD_STATE_INIT))
        attr->value.u32 = TAI_NETWORK_INTERFACE_OPER_STATUS_INITIALIZE;
    else
        attr->value.u32 = TAI_NETWORK_INTERFACE_OPER_STATUS_UNKNOWN;

    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Set the operational status of the module
 *
 * @param [in] module_id The module identifier
 * @param [in,out] attr The attribute which contains the operational status to
 *        set.
 *
 * @return tai_status_t
 */
tai_status_t ac400_set_module_oper_status(
   _In_ tai_object_id_t  module_id,
   _In_ const tai_attribute_t *attr)
{
    bool modRst   = false;
    bool modLoPwr = false;
    bool modTxDis = false;
    tai_attribute_t prev_state;
    tai_attribute_t curr_state;
    int reps;
    bool glb_alrm;
    int waitTime;
    int sleepTime;

    /* Get the current module state */
    prev_state.id = TAI_NETWORK_INTERFACE_ATTR_OPER_STATUS;
    if (TAI_STATUS_SUCCESS != ac400_get_module_oper_status(module_id, &prev_state)) {
        return TAI_STATUS_FAILURE;
    }

    /* Figure out which signals to set/clear */
    switch (attr->value.u32) {
        case TAI_NETWORK_INTERFACE_OPER_STATUS_RESET:
            modRst = true;   /* fallthrough */
        case TAI_NETWORK_INTERFACE_OPER_STATUS_LOW_POWER:
            modLoPwr = true; /* fallthrough */
        case TAI_NETWORK_INTERFACE_OPER_STATUS_TX_OFF:
            modTxDis = true; /* fallthrough */
        case TAI_NETWORK_INTERFACE_OPER_STATUS_READY:
            break;
        default:
            return TAI_STATUS_INVALID_ATTRIBUTE_0;
    }

    /* Change the module state */
    if (ac400_set_reset(module_id, modRst)) {
        return TAI_STATUS_FAILURE;
    }
    if (ac400_set_mod_lopwr(module_id, modLoPwr)) {
        return TAI_STATUS_FAILURE;
    }
    if (ac400_set_txdis(module_id, 1, modTxDis)) {
        return TAI_STATUS_FAILURE;
    }
    if (ac400_set_txdis(module_id, 2, modTxDis)) {
        return TAI_STATUS_FAILURE;
    }

    /* If coming out of reset, Wait until global alarm is asserted */
    if (prev_state.value.u32 == TAI_NETWORK_INTERFACE_OPER_STATUS_RESET &&
        attr->value.u32 != TAI_NETWORK_INTERFACE_OPER_STATUS_RESET) {
        reps = 100; 
        glb_alrm = false;
        do {
            ac400_get_glb_alrm(module_id, &glb_alrm);
            if (glb_alrm)
                break;
            usleep(200000);
        }
        while (--reps);
        if (!reps) {
            TAI_SYSLOG_ERROR("Timed out waiting for module to complete reset");
            return TAI_STATUS_FAILURE;
        }
    }

    /* Wait until the module enters the requested state */
    sleepTime = 25;
    waitTime = ac400_get_transition_time(prev_state.value.u32, attr->value.u32) * 1000000;
    do {
        /* First time through, don't sleep. */
        if (25 != sleepTime) {
            usleep(sleepTime);
            waitTime -= sleepTime;
        }
        /* Double sleep time each loop, with a 1/2 sec max */
        sleepTime = (sleepTime*2 < 500000) ? sleepTime*2 : 500000;

        curr_state.id = TAI_NETWORK_INTERFACE_ATTR_OPER_STATUS;
        if (TAI_STATUS_SUCCESS != ac400_get_module_oper_status(module_id, &curr_state)) {
            return TAI_STATUS_FAILURE;
        }
        if (curr_state.value.u32 == attr->value.u32) {
            return TAI_STATUS_SUCCESS;
        }
    }
    while (waitTime > 0);

    return TAI_STATUS_FAILURE;
}

/**
 * @brief Retrieve the operational mode of the network interfaces
 *
 * @param [in] module_id The AC400 module identifier
 * @param [in,out] attr The attribute into which the operational mode (enum) is
 *        placed
 *
 * @return tai_status_t
 */
tai_status_t ac400_get_network_mode(
   _In_ tai_object_id_t     module_id,
   _Inout_ tai_attribute_t *attr)
{
    uint16_t word;

    if (ac400_mdio_read(module_id, AC400_DEVICE_SETUP_CNTL_REG, &word)) {
        return TAI_STATUS_FAILURE;
    }

    switch (AC400_GET_FIELD(word, AC400_DEVICE_SETUP_CNTL_DEV_CFG)) {
        case 0:
            attr->value.u32 = TAI_MODULE_NETWORK_MODE_INDEPENDENT;
            break;
        case 1:
            attr->value.u32 = TAI_MODULE_NETWORK_MODE_COUPLED;
            break;
        default:
            attr->value.u32 = TAI_MODULE_NETWORK_MODE_UNKNOWN;
            break;
    }

    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Set the operational mode of the network interfaces
 *
 * @param [in] module_id The AC400 module identifier
 * @param [in,out] attr The attribute which contains the mode to set.
 *
 * @return tai_status_t
 */
static tai_status_t ac400_set_network_mode(
   _In_ tai_object_id_t  module_id,
   _In_ const tai_attribute_t *attr)
{
    uint16_t mode;
    tai_status_t ret;
    tai_attribute_t prev_state;
    tai_attribute_t lopwr_state;

    switch (attr->value.u32) {
        case TAI_MODULE_NETWORK_MODE_INDEPENDENT:
            mode = 0;
            break;
        case TAI_MODULE_NETWORK_MODE_COUPLED:
            mode = 1;
            break;
        default:
            return TAI_STATUS_INVALID_ATTRIBUTE_0;
    }

    ret = ac400_get_module_oper_status(module_id, &prev_state);
    if (TAI_STATUS_SUCCESS != ret) {
        return ret;
    }
    if (TAI_NETWORK_INTERFACE_OPER_STATUS_LOW_POWER != prev_state.value.u32) {
        lopwr_state.value.u32 = TAI_NETWORK_INTERFACE_OPER_STATUS_LOW_POWER;
        ret = ac400_set_module_oper_status(module_id, &lopwr_state);
        if (TAI_STATUS_SUCCESS != ret) {
            return ret;
        }
    }
    if (ac400_mdio_write(module_id, AC400_DEVICE_SETUP_CNTL_REG, mode)) {
        return TAI_STATUS_FAILURE;
    }
    if (TAI_NETWORK_INTERFACE_OPER_STATUS_LOW_POWER != prev_state.value.u32) {
        ret = ac400_set_module_oper_status(module_id, &prev_state);
        if (TAI_STATUS_SUCCESS != ret) {
            return ret;
        }
    }

    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Retrieve the temperature of the module, in degrees celsius
 *
 * @param [in] module_id The AC400 module identifier
 * @param [in,out] attr The attribute into which the module temperature (float)
 *        is placed
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_module_temp(
   _In_ tai_object_id_t     module_id,
   _Inout_ tai_attribute_t *attr)
{
    uint16_t word;

    if (ac400_mdio_read(module_id, AC400_TEMP_MON_A2D_VAL_REG, &word)) {
        return TAI_STATUS_FAILURE;
    }
    attr->value.flt = (int16_t) word / 256.0;

    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Retrieve the power (voltage) supplied to the module
 *
 * @param [in] module_id The AC400 module identifier
 * @param [in,out] attr The attribute into which the module power (float) is
 *        placed
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_module_power(
   _In_ tai_object_id_t     module_id,
   _Inout_ tai_attribute_t *attr)
{
    uint16_t word;

    if (ac400_mdio_read(module_id, AC400_MOD_PS_MON_A2D_VAL_REG, &word)) {
        return TAI_STATUS_FAILURE;
    }
    attr->value.flt = word / 1000.0;

    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Retrieve the value of an attribute
 *
 * @param [in] module_id The module ID handle
 * @param [in,out] attr A pointer to the attribute to be retrieved
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t ac400_get_module_attribute(
    _In_ tai_object_id_t     module_id,
    _Inout_ tai_attribute_t *attr)
{
    char location[TAI_MAX_HARDWARE_ID_LEN+1];
    uint32_t len;

    TAI_SYSLOG_DEBUG("Retrieving module attribute: %d", attr->id);
    switch (attr->id) {
        case TAI_MODULE_ATTR_LOCATION:
            ac400_get_module_location(module_id, location);
            len = strlen(location);
            if (attr->value.charlist.count < len+1) {
                attr->value.charlist.count = len+1;
                return TAI_STATUS_BUFFER_OVERFLOW;
            }
            attr->value.charlist.count = len+1;
            strcpy(attr->value.charlist.list, location);
            return TAI_STATUS_SUCCESS;
        case TAI_MODULE_ATTR_VENDOR_NAME:
            return ac400_get_string(module_id, attr, AC400_VEND_NAME_BYTE_1_REG, 16);
        case TAI_MODULE_ATTR_VENDOR_PART_NUMBER:
            return ac400_get_string(module_id, attr, AC400_VEND_PN_BYTE_1_REG, 16);
        case TAI_MODULE_ATTR_VENDOR_SERIAL_NUMBER:
            return ac400_get_string(module_id, attr, AC400_VEND_SN_BYTE_1_REG, 16);
        case TAI_MODULE_ATTR_FIRMWARE_VERSIONS:
            return ac400_get_fw_versions(module_id, attr);
        case TAI_MODULE_ATTR_OPER_STATUS:
            return ac400_get_tai_oper_status(module_id, attr);
        case TAI_MODULE_ATTR_NETWORK_MODE:
            return ac400_get_network_mode(module_id, attr);
        case TAI_MODULE_ATTR_TEMP:
            return ac400_get_module_temp(module_id, attr);
        case TAI_MODULE_ATTR_POWER:
            return ac400_get_module_power(module_id, attr);
        case TAI_MODULE_ATTR_NUM_HOST_INTERFACES:
            attr->value.u32 = AC400_NUM_HOSTIF;
            return TAI_STATUS_SUCCESS;
        case TAI_MODULE_ATTR_NUM_NETWORK_INTERFACES:
            attr->value.u32 = AC400_NUM_NETIF;
            return TAI_STATUS_SUCCESS;
    }
    return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
}

/**
 * @brief Retrieve a list of attribute values
 *
 * @param [in] module_id The module ID handle
 * @param [in] attr_count A count of the number of elements in the attr_list
 * @param [in,out] attr_list A list of attributes to be retrieved
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t ac400_get_module_attributes(
    _In_ tai_object_id_t     module_id,
    _In_ uint32_t            attr_count,
    _Inout_ tai_attribute_t *attr_list)
{
    uint32_t idx;
    tai_status_t ret;

    for (idx = 0; idx < attr_count; idx++) {
        ret = ac400_get_module_attribute(module_id, attr_list++);
        if (ret) {
            return convert_tai_error_to_list(ret, idx);
        }
    }
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Set the value of an attribute
 *
 * @param [in] module_id The module ID handle
 * @param [in] attr A pointer to the attribute to be set
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t ac400_set_module_attribute(
   _In_ tai_object_id_t        module_id,
   _In_ const tai_attribute_t *attr)
{
    TAI_SYSLOG_DEBUG("Setting module attribute: %d", attr->id);
    switch (attr->id) {
        case TAI_MODULE_ATTR_LOCATION:
            return TAI_STATUS_SUCCESS;
        case TAI_MODULE_ATTR_VENDOR_NAME:
        case TAI_MODULE_ATTR_VENDOR_PART_NUMBER:
        case TAI_MODULE_ATTR_VENDOR_SERIAL_NUMBER:
        case TAI_MODULE_ATTR_FIRMWARE_VERSIONS:
        case TAI_MODULE_ATTR_TEMP:
        case TAI_MODULE_ATTR_POWER:
        case TAI_MODULE_ATTR_NUM_HOST_INTERFACES:
        case TAI_MODULE_ATTR_NUM_NETWORK_INTERFACES:
        case TAI_MODULE_ATTR_OPER_STATUS:
            return TAI_STATUS_INVALID_ATTRIBUTE_0;
        case TAI_MODULE_ATTR_NETWORK_MODE:
            return ac400_set_network_mode(module_id, attr);
    }
    return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
}

/**
 * @brief Set the values from a list of attributes
 *
 * @param [in] module_id The module ID handle
 * @param [in] attr_count A count of the number of elements in the attr_list
 * @param [in] attr_list A list of attributes to be set
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t ac400_set_module_attributes(
   _In_ tai_object_id_t        module_id,
   _In_ uint32_t               attr_count,
   _In_ const tai_attribute_t *attr_list)
{
    uint32_t idx;
    tai_status_t ret;

    for (idx = 0; idx < attr_count; idx++) {
        ret = ac400_set_module_attribute(module_id, attr_list++);
        if (ret) {
            return convert_tai_error_to_list(ret, idx);
        }
    }
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Module initialization. After the call the capability attributes should
 *        be ready for retrieval via tai_get_module_attribute().
 *
 * @param [out] module_id Handle which identifies the module
 * @param [in] attr_count A count of the number of elements in the attr_list
 * @param [in] attr_list A list of attributes to set during initialization
 * @param [in] notifications Function pointers for adapter host notifications
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t ac400_create_module(
    _Out_ tai_object_id_t          *module_id,
    _In_ uint32_t                   attr_count,
    _In_ const tai_attribute_t     *attr_list,
    _In_ tai_module_notification_t *notifications)
{
    tai_status_t ret;
    const tai_attribute_value_t * mod_addr;
    ac400_object_id_t *ac400_obj = (ac400_object_id_t *)module_id;

    if (NULL == notifications) {
        TAI_SYSLOG_ERROR("NULL module notifications passed to TAI switch initialize");
        return TAI_STATUS_INVALID_PARAMETER;
    }

    mod_addr = find_attribute_in_list(TAI_MODULE_ATTR_LOCATION, attr_count, attr_list);
    if (NULL == mod_addr) {
        TAI_SYSLOG_ERROR("The required TAI_MODULE_ATTR_LOCATION attribute was not provided");
        return TAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    ret = ac400_set_module_id(mod_addr, module_id);
    if (ret) {
        TAI_SYSLOG_ERROR("Invalid TAI_MODULE_ATTR_LOCATION attribute value.");
        return TAI_STATUS_INVALID_PARAMETER;
    }

    ret = ac400_module_init(*module_id);
    if (TAI_STATUS_SUCCESS != ret) {
        TAI_SYSLOG_ERROR("Module initialization failed");
        return ret;
    }

    module_init[ac400_obj->value - 1] = true;

    ret = ac400_set_module_attributes(*module_id, attr_count, attr_list);
    if (TAI_STATUS_SUCCESS != ret) {
        TAI_SYSLOG_ERROR("Error setting module attributes");
        return ret;
    }

    memcpy(&notification_callbacks, notifications, sizeof(notification_callbacks));

    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Release all resources associated with previously created module
 *
 * @param [in] module_id The module ID handle being removed
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t ac400_remove_module(_In_ tai_object_id_t module_id)
{
    ac400_object_id_t *ac400_obj = (ac400_object_id_t *)&module_id;

    module_init[ac400_obj->value - 1] = false;
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief The module interface functions. This structure is retrieved via the
 *        #tai_api_query function.
 */
tai_module_api_t ac400_module_api = {
    .create_module         = ac400_create_module,
    .remove_module         = ac400_remove_module,
    .set_module_attribute  = ac400_set_module_attribute,
    .set_module_attributes = ac400_set_module_attributes,
    .get_module_attribute  = ac400_get_module_attribute,
    .get_module_attributes = ac400_get_module_attributes
};

