/**
 *  @file    ac400_voyager.c
 *  @brief   Interface routes to the AC400 on a Voyager platform
 *  @author  Scott Emery <scotte@cumulusnetworks.com>
 *
 *  This file contains all of the routines which interface to the AC400 on the
 *  Voyager platform. Any code which is specific to the Voyager platform is
 *  included in this file.
 *
 *  @copyright Copyright (C) 2018 Cumulus Networks, Inc. All rights reserved
 *
 *  @remark  This source code is licensed under the BSD 3-Clause license found
 *           in the LICENSE file in the root directory of this source tree.
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include "voyager_tai_adapter.h"

#undef  __TAI_MODULE__
#define __TAI_MODULE__ TAI_API_UNSPECIFIED

static char fname[PATH_MAX];
static pthread_mutex_t  mod_abs_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief Get the value of a given tai object id. Does no error checking.
 *
 * @param obj_id [in] The object ID
 *
 * @return int The value field from the object id
 */
static int get_obj_val(_In_ tai_object_id_t obj_id)
{
    ac400_object_id_t *ac400_obj = (ac400_object_id_t *)&obj_id;
    return ac400_obj->value;
}

/**
 *  @brief Read the value of an integer from a file. The file is expected to
 *         contain only either a decimal, hexadecimal, or octal value.
 *
 *  @param [in] file The name of the file to read
 *  @param [out] value A pointer to an integer into which the value will be
 *         placed.
 *
 *  @return 0 on success, or a Linux error code on failure.
 */
static int read_int_from_file(_In_ char * file, _Out_ int *value)
{
    FILE *fp;
    int ret;

    fp = fopen(file, "r");
    if (NULL == fp) {
        return EIO;
    }

    ret = fscanf(fp, "%i", value);
    fclose(fp);
    return (1 == ret) ? 0 : EIO;
}

/**
 *  @brief Read the value of a signal from a file. The file is expected to
 *         contain either '0' or '1' as the first character.
 *
 *  @param [in] file The name of the file to read
 *  @param [out] signal A pointer to a boolean into which the signals value (the
 *         first character of the file) will be placed.
 *
 *  @return 0 on success, or a Linux error code on failure.
 */
static int read_signal_from_file(_In_ char * file, _Out_ bool *signal)
{
    int inval;
    int ret;

    ret = read_int_from_file(file, &inval);
    *signal = (inval != 0);
    return ret;
}

/**
 *  @brief Write the value of an integer to a file.
 *
 *  @param [in] file The name of the file to write
 *  @param [in] value An integer to write to the file
 *
 *  @return 0 on success, or a Linux error code on failure.
 */
static int write_int_to_file(_In_ char * file, _In_ int value)
{
    FILE *fp;
    int ret;

    fp = fopen(file, "w");
    if (NULL == fp) {
        return EIO;
    }

    ret = fprintf(fp, "%d", value);
    fclose(fp);
    return (ret >= 0) ? 0 : EIO;
}

/**
 *  @brief Write the value of a signal to a file. Either '0' or '1' is written
 *         to the file.
 *
 *  @param [in] file The name of the file to write
 *  @param [in] signal A boolean which specifies the value of the signal to be
 *         written to the file, false == '0' and true == '1'
 *
 *  @return 0 on success, or a Linux error code on failure.
 */
static int write_signal_to_file(_In_ char * file, _In_ bool signal)
{
    int inval = (signal) ? 1 : 0;
    return write_int_to_file(file, inval);
}

/**
 *  @brief Return the value of the global alarm signal
 *
 *  @param [in] module_id The module ID of the module being accessed
 *  @param [out] glb_alrm A pointer to a bool to place the current global alarm
 *         value.
 *
 *  @return 0 on success, or a Linux error code on failure.
 */
int ac400_get_glb_alrm(
   _In_ tai_object_id_t     module_id,
   _Out_ bool              *glb_alrm)
{
    snprintf(fname, PATH_MAX, "%s/ac400_%d_glb_alarm", CPLD_PATH,
             get_obj_val(module_id));
    return read_signal_from_file(fname, glb_alrm);
}

/**
 *  @brief Return the value of the RXLOS signal
 *
 *  @param [in] module_id The module ID of the module being accessed
 *  @param [in] networkif The number of the network interface, 1 or 2
 *  @param [out] rxlos A pointer to a bool to place the current rxlos value.
 *
 *  @return 0 on success, or a Linux error code on failure.
 */
int ac400_get_rxlos(
   _In_ tai_object_id_t     module_id,
   _In_ int                 networkif,
   _Out_ bool              *rxlos)
{
    snprintf(fname, PATH_MAX, "%s/ac400_%d_rxlos%d", CPLD_PATH,
             get_obj_val(module_id), networkif);
    return read_signal_from_file(fname, rxlos);
}

/**
 *  @brief Return the value of the MOD_ABS ignal
 *
 *  @param [in] module_id The module ID of the module being accessed
 *  @param [out] mod_abs A pointer to a bool to place the current mod_abs value.
 *
 *  @return 0 on success, or a Linux error code on failure.
 */
int ac400_get_mod_abs(
   _In_ tai_object_id_t     module_id,
   _Out_ bool              *mod_abs)
{
    char fname[PATH_MAX];
    int ret;

    pthread_mutex_lock(&mod_abs_mutex);
    snprintf(fname, PATH_MAX, "%s/ac400_%d_mod_absent", CPLD_PATH,
             get_obj_val(module_id));
    ret = read_signal_from_file(fname, mod_abs);
    pthread_mutex_unlock(&mod_abs_mutex);
    return ret;
}

/**
 *  @brief Return the value of the TXDIS signal
 *
 *  @param [in] module_id The module ID of the module being accessed
 *  @param [in] networkif The number of the network interface, 1 or 2
 *  @param [out] txdis A pointer to a bool to place the current txdis value.
 *
 *  @return 0 on success, or a Linux error code on failure.
 */
int ac400_get_txdis(
   _In_ tai_object_id_t     module_id,
   _In_ int                 networkif,
   _Out_ bool              *txdis)
{
    snprintf(fname, PATH_MAX, "%s/ac400_%d_tx_disable%d", CPLD_PATH,
             get_obj_val(module_id), networkif-1);
    return read_signal_from_file(fname, txdis);
}

/**
 *  @brief Set the value of the TXDIS signal
 *
 *  @param [in] module_id The module ID of the module being accessed
 *  @param [in] networkif The number of the network interface, 1 or 2
 *  @param [in] txdis The value to set the txdis signal
 *
 *  @return 0 on success, or a Linux error code on failure.
 */
int ac400_set_txdis(
   _In_ tai_object_id_t     module_id,
   _In_ int                 networkif,
   _In_ bool                txdis)
{
    snprintf(fname, PATH_MAX, "%s/ac400_%d_tx_disable%d", CPLD_PATH,
             get_obj_val(module_id), networkif-1);
    return write_signal_to_file(fname, txdis);
}

/**
 *  @brief Return the value of the MOD_LOPWR signal
 *
 *  @param [in] module_id The module ID of the module being accessed
 *  @param [out] mod_lopwr A pointer to a bool to place the current mod_lopwr
 *         value.
 *
 *  @return 0 on success, or a Linux error code on failure.
 */
int ac400_get_mod_lopwr(
   _In_ tai_object_id_t     module_id,
   _Out_ bool              *mod_lopwr)
{
    snprintf(fname, PATH_MAX, "%s/ac400_%d_mod_lopwr", CPLD_PATH,
             get_obj_val(module_id));
    return read_signal_from_file(fname, mod_lopwr);
}

/**
 *  @brief Set the value of the MOD_LOPWR signal
 *
 *  @param [in] module_id The module ID of the module being accessed
 *  @param [in] mod_lopwr The value to set the mod_lopwr signal
 *
 *  @return 0 on success, or a Linux error code on failure.
 */
int ac400_set_mod_lopwr(
   _In_ tai_object_id_t     module_id,
   _In_ bool                mod_lopwr)
{
    snprintf(fname, PATH_MAX, "%s/ac400_%d_mod_lopwr", CPLD_PATH,
             get_obj_val(module_id));
    return write_signal_to_file(fname, mod_lopwr);
}

/**
 *  @brief Return the value of the PM_SYNC signal
 *
 *  @param [in] module_id The module ID of the module being accessed
 *  @param [out] pm_sync A pointer to a bool to place the current pm_sync value.
 *
 *  @return 0 on success, or a Linux error code on failure.
 */
int ac400_get_pm_sync(
   _In_ tai_object_id_t     module_id,
   _Out_ bool              *pm_sync)
{
    snprintf(fname, PATH_MAX, "%s/ac400_%d_pm_sync", CPLD_PATH,
             get_obj_val(module_id));
    return read_signal_from_file(fname, pm_sync);
}

/**
 *  @brief Set the value of the PM_SYNC signal
 *
 *  @param [in] module_id The module ID of the module being accessed
 *  @param [in] pm_sync The value to set the pm_sync signal
 *
 *  @return 0 on success, or a Linux error code on failure.
 */
int ac400_set_pm_sync(
   _In_ tai_object_id_t     module_id,
   _In_ bool                pm_sync)
{
    snprintf(fname, PATH_MAX, "%s/ac400_%d_pm_sync", CPLD_PATH,
             get_obj_val(module_id));
    return write_signal_to_file(fname, pm_sync);
}

/**
 *  @brief Return the value of the RESET signal
 *
 *  @param [in] module_id The module ID of the module being accessed
 *  @param [out] reset A pointer to a bool to place the current reset value.
 *
 *  @return 0 on success, or a Linux error code on failure.
 */
int ac400_get_reset(
   _In_ tai_object_id_t     module_id,
   _Out_ bool              *reset)
{
    snprintf(fname, PATH_MAX, "%s/reset_ac400_%d", CPLD_PATH,
             get_obj_val(module_id));
    return read_signal_from_file(fname, reset);
}

/**
 *  @brief Set the value of the RESET signal
 *
 *  @param [in] module_id The module ID of the module being accessed
 *  @param [in] reset The value to set the reset signal
 *
 *  @return 0 on success, or a Linux error code on failure.
 */
int ac400_set_reset(
   _In_ tai_object_id_t     module_id,
   _In_ bool                reset)
{
    snprintf(fname, PATH_MAX, "%s/reset_ac400_%d", CPLD_PATH,
             get_obj_val(module_id));
    return write_signal_to_file(fname, reset);
}

/**
 *  @brief Return the value of the POWER signal
 *
 *  @param [in] module_id The module ID of the module being accessed
 *  @param [out] power A pointer to a bool to place the current power value.
 *
 *  @return 0 on success, or a Linux error code on failure.
 */
int ac400_get_power(
   _In_ tai_object_id_t     module_id,
   _Out_ bool              *power)
{
    snprintf(fname, PATH_MAX, "%s/pwr_ac400_%d", CPLD_PATH,
             get_obj_val(module_id));
    return read_signal_from_file(fname, power);
}

/**
 *  @brief Set the value of the POWER signal
 *
 *  @param [in] module_id The module ID of the module being accessed
 *  @param [in] power The value to set the power signal
 *
 *  @return 0 on success, or a Linux error code on failure.
 */
int ac400_set_power(
   _In_ tai_object_id_t     module_id,
   _In_ bool                power)
{
    snprintf(fname, PATH_MAX, "%s/pwr_ac400_%d", CPLD_PATH,
             get_obj_val(module_id));
    return write_signal_to_file(fname, power);
}

/**
 *  @brief Return the value of the MDIO device_type
 *
 *  @param [out] type A pointer to an int to place the current MDIO device_type.
 *
 *  @return 0 on success, or a Linux error code on failure.
 */
int ac400_get_device_type(
   _Out_ int               *type)
{
    snprintf(fname, PATH_MAX, "%s/mdio_device_type", CPLD_PATH);
    return read_int_from_file(fname, type);
}

/**
 *  @brief Set the value of the MDIO device_type
 *
 *  @param [in] type The value to set the MDIO device type
 *
 *  @return 0 on success, or a Linux error code on failure.
 */
int ac400_set_device_type(
   _In_ int                 type)
{
    snprintf(fname, PATH_MAX, "%s/mdio_device_type", CPLD_PATH);
    return write_int_to_file(fname, type);
}

/**
 *  @brief Perform an MDIO read from the AC400 module
 *
 *  @param [in] module_id The module ID of the module being accessed
 *  @param [in] reg The MDIO address of the register to read
 *  @param [out] value A pointer to a u16 to place the value read from the AC400
 *
 *  @return 0 on success, or a Linux error code on failure.
 */
int ac400_mdio_read(
   _In_ tai_object_id_t     module_id,
   _In_ uint16_t            reg,
   _Out_ uint16_t          *value)
{
    int ret;
    int intval;

    snprintf(fname, PATH_MAX, "%s/mdio_%d_address", CPLD_PATH,
             get_obj_val(module_id));
    ret = write_int_to_file(fname, reg);
    if (ret) {
        return ret;
    }
    snprintf(fname, PATH_MAX, "%s/mdio_%d_data", CPLD_PATH,
             get_obj_val(module_id));
    ret = read_int_from_file(fname, &intval);
    *value = (uint16_t)intval;

    if (reg != 0xb016) {
        TAI_SYSLOG_DEBUG("ac400_mdio_read:  module %d reg 0x%04x value 0x%04x", get_obj_val(module_id), reg, *value); 
    }
    return ret;
}

/**
 *  @brief Perform an MDIO write to the AC400 module
 *
 *  @param [in] module_id The module ID of the module being accessed
 *  @param [in] reg The MDIO address of the register to write
 *  @param [out] value A u16 value to write to the AC400
 *
 *  @return 0 on success, or a Linux error code on failure.
 */
int ac400_mdio_write(
   _In_ tai_object_id_t     module_id,
   _In_ uint16_t            reg,
   _In_ uint16_t            value)
{
    int ret;

    TAI_SYSLOG_DEBUG("ac400_mdio_write: module %d reg 0x%04x value 0x%04x", get_obj_val(module_id), reg, value);
    snprintf(fname, PATH_MAX, "%s/mdio_%d_address", CPLD_PATH,
             get_obj_val(module_id));
    ret = write_int_to_file(fname, reg);
    if (ret) {
        return ret;
    }
    snprintf(fname, PATH_MAX, "%s/mdio_%d_data", CPLD_PATH,
             get_obj_val(module_id));
    return write_int_to_file(fname, value);
}

/**
 * @brief Validate the module location and initialize the module_id handle.
 *
 * @param [in] mod_addr A pointer to a #tai_char_list_t (string) which gives the
 *        module index, either "1" or "2".
 * @param [out] module_id A pointer to the object id for the module object.
 *
 * @return 0 on success, or -1 on failure.
 */
int ac400_set_module_id(_In_ const tai_attribute_value_t * mod_addr,
                        _Out_ tai_object_id_t            * module_id)
{
    ac400_object_id_t *ac400_obj = (ac400_object_id_t *)module_id;

    if (!strcmp(mod_addr->charlist.list, "1")) {
        ac400_obj->type = TAI_OBJECT_TYPE_MODULE;
        ac400_obj->value = 1;
    }
    else if (!strcmp(mod_addr->charlist.list, "2")) {
        ac400_obj->type = TAI_OBJECT_TYPE_MODULE;
        ac400_obj->value = 2;
    }
    else {
        TAI_SYSLOG_ERROR("Invalid Module Location value for Voyager.");
        return -1;
    }
    return 0;
}

/**
 * @brief Given a module_id, return the module location.
 *
 * @param [in] module_id A module object id.
 * @param [out] location A pointer to location where the module location will be
 *        stored.
 */
void ac400_get_module_location(_In_ tai_object_id_t module_id,
                              _Out_ char         * location)
{
    ac400_object_id_t *ac400_obj = (ac400_object_id_t *)&module_id;
    sprintf(location, "%d", ac400_obj->value);
}
