/**
 *  @file    ac400_tai_hostif.c
 *  @brief   The TAI host interface routines
 *  @author  Scott Emery <scotte@cumulusnetworks.com>
 *
 *  @copyright Copyright (C) 2018 Cumulus Networks, Inc. All rights reserved
 *
 *  @remark  This source code is licensed under the BSD 3-Clause license found
 *           in the LICENSE file in the root directory of this source tree.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "voyager_tai_adapter.h"

#undef  __TAI_MODULE__
#define __TAI_MODULE__ TAI_API_HOSTIF

typedef struct _host_interface_info_t {
    bool            initialized;
    tai_object_id_t module_id;  /* module handle */
    int             hostif_idx; /* zero-based index on the module */
} host_interface_info_t;

static host_interface_info_t hostif_info[VOYAGER_NUM_HOSTIF];

/**
 * @brief Retrieve the lane faults list
 *
 * @param [in] host_interface_id The host interface identifier
 * @param [in,out] attr The attribute into which the lane fault status will be
 *        placed (#tai_u32_list_t of #tai_host_interface_lane_faults_t)
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_lane_faults(
   _In_ tai_object_id_t     host_interface_id,
   _Inout_ tai_attribute_t *attr)
{
    ac400_object_id_t *hostif_obj = (ac400_object_id_t *)&host_interface_id;
    tai_object_id_t module_id = hostif_info[hostif_obj->value].module_id;
    tai_host_interface_lane_faults_t *cp;
    uint16_t reg_addr = AC400_HOST_FLT_STAT_LN_0_REG +
        AC400_NUM_HOST_LANES*hostif_info[hostif_obj->value].hostif_idx;
    uint32_t num_lanes = AC400_NUM_HOST_LANES;
    uint16_t word1;

    /* Is there enough room in the attribute array? */
    if (attr->value.u32list.count < num_lanes) {
        attr->value.u32list.count = num_lanes;
        return TAI_STATUS_BUFFER_OVERFLOW;
    }
    attr->value.u32list.count = num_lanes;
    cp = attr->value.u32list.list;

    while (num_lanes--) {
        if (ac400_mdio_read(module_id, reg_addr++, &word1)) {
            return TAI_STATUS_FAILURE;
        }
        *cp = (word1 & AC400_HOST_FLT_STAT_LN_TX_HOST_LOL_BIT) ?
            TAI_HOST_INTERFACE_LANE_FAULT_LOSS_OF_LOCK : 0;
        *cp++ |= (word1 & 0x0002) ?
            TAI_HOST_INTERFACE_LANE_FAULT_TX_FIFIO_ERR : 0;
    }
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Retrieve the TX alignment status
 *
 * @param [in] host_interface_id The host interface identifier
 * @param [in,out] attr The attribute into which the client interface tx
 *        alignment status is placed (#tai_host_interface_tx_align_status_t).
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_tx_align(
   _In_ tai_object_id_t     host_interface_id,
   _Inout_ tai_attribute_t *attr)
{
    ac400_object_id_t *hostif_obj = (ac400_object_id_t *)&host_interface_id;
    tai_object_id_t module_id = hostif_info[hostif_obj->value].module_id;
    uint16_t word1;

    if (ac400_mdio_read(module_id, AC400_CLIENT_TX_ALGN_STAT_INTF_0_REG +
                        hostif_info[hostif_obj->value].hostif_idx, &word1)) {
        return TAI_STATUS_FAILURE;
    }
    attr->value.u32 = (word1 & AC400_CLIENT_TX_ALGN_STAT_INTF_LOSS_OF_ALGN_BIT) ?
        TAI_HOST_INTERFACE_TX_ALIGN_LOSS : 0;
    attr->value.u32 |= (word1 & AC400_CLIENT_TX_ALGN_STAT_INTF_OUT_OF_ALGN_BIT) ?
        TAI_HOST_INTERFACE_TX_ALIGN_OUT : 0;
    attr->value.u32 |= (word1 & AC400_CLIENT_TX_ALGN_STAT_INTF_DESKW_LCK_FLT_BIT) ?
        TAI_HOST_INTERFACE_TX_ALIGN_DESKEW_LOCK : 0;

    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Retrieve the interface rate
 *
 * @param [in] host_interface_id The host interface identifier
 * @param [in,out] attr The attribute into which the client interface rate is
 *        placed (#tai_host_interface_rate_t).
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_rate(
   _In_ tai_object_id_t     host_interface_id,
   _Inout_ tai_attribute_t *attr)
{
    ac400_object_id_t *hostif_obj = (ac400_object_id_t *)&host_interface_id;
    tai_object_id_t module_id = hostif_info[hostif_obj->value].module_id;
    uint16_t word1;
    uint16_t ac400_rate;

    if (ac400_mdio_read(module_id, AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_REG, &word1)) {
        return TAI_STATUS_FAILURE;
    }

    switch (hostif_info[hostif_obj->value].hostif_idx) {
        case 0:
            ac400_rate = AC400_GET_FIELD(word1, AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_INTF_0_LANES_0_TO_3_RATE_SEL);
            break;
        case 1:
            ac400_rate = AC400_GET_FIELD(word1, AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_INTF_1_LANES_4_TO_7_RATE_SEL);
            break;
        case 2:
            ac400_rate = AC400_GET_FIELD(word1, AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_INTF_2_LANES_8_TO_11_RATE_SEL);
            break;
        case 3:
            ac400_rate = AC400_GET_FIELD(word1, AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_INTF_3_LANES_12_TO_15_RATE_SEL);
            break;
        default:
            return TAI_STATUS_FAILURE;
    }

    switch (ac400_rate) {
        case 0:
            attr->value.u32 = TAI_HOST_INTERFACE_RATE_OTU4_27_95G;
            break;
        case 1:
            attr->value.u32 = TAI_HOST_INTERFACE_RATE_100GE_25_78G;
            break;
        default:
            return TAI_STATUS_FAILURE;
    }

    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Set the interface rate
 *
 * @param [in] host_interface_id The host interface identifier
 * @param [in] attr The attribute which contains the client interface rate
 *        (#tai_host_interface_rate_t).
 *
 * @return tai_status_t
 */
static tai_status_t ac400_set_rate(
   _In_ tai_object_id_t        host_interface_id,
   _In_ const tai_attribute_t *attr)
{
    ac400_object_id_t *hostif_obj = (ac400_object_id_t *)&host_interface_id;
    tai_object_id_t module_id = hostif_info[hostif_obj->value].module_id;
    uint16_t word1;
    uint16_t ac400_rate;

    switch (attr->value.u32) {
        case TAI_HOST_INTERFACE_RATE_OTU4_27_95G:
            ac400_rate = 0;
            break;
        case TAI_HOST_INTERFACE_RATE_100GE_25_78G:
            ac400_rate = 1;
            break;
        default:
            return TAI_STATUS_FAILURE;
    }

    if (ac400_mdio_read(module_id, AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_REG, &word1)) {
        return TAI_STATUS_FAILURE;
    }

    switch (hostif_info[hostif_obj->value].hostif_idx) {
        case 0:
            word1 = AC400_PUT_FIELD(word1, AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_INTF_0_LANES_0_TO_3_RATE_SEL, ac400_rate);
            break;
        case 1:
            word1 = AC400_PUT_FIELD(word1, AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_INTF_1_LANES_4_TO_7_RATE_SEL, ac400_rate);
            break;
        case 2:
            word1 = AC400_PUT_FIELD(word1, AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_INTF_2_LANES_8_TO_11_RATE_SEL, ac400_rate);
            break;
        case 3:
            word1 = AC400_PUT_FIELD(word1, AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_INTF_3_LANES_12_TO_15_RATE_SEL, ac400_rate);
            break;
        default:
            return TAI_STATUS_FAILURE;
    }

    if (ac400_mdio_write(module_id, AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_REG, word1)) {
        return TAI_STATUS_FAILURE;
    }

    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Retrieve the interface enable state
 *
 * @param [in] host_interface_id The host interface identifier
 * @param [in,out] attr The attribute into which the client interface enable
 *        state is placed (bool).
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_enable(
   _In_ tai_object_id_t     host_interface_id,
   _Inout_ tai_attribute_t *attr)
{
    ac400_object_id_t *hostif_obj = (ac400_object_id_t *)&host_interface_id;
    tai_object_id_t module_id = hostif_info[hostif_obj->value].module_id;
    uint16_t word1;
    uint16_t enable;

    if (ac400_mdio_read(module_id, AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_REG, &word1)) {
        return TAI_STATUS_FAILURE;
    }

    switch (hostif_info[hostif_obj->value].hostif_idx) {
        case 0:
            enable = AC400_GET_BIT(word1, AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_INTF_0_EN);
            break;
        case 1:
            enable = AC400_GET_BIT(word1, AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_INTF_1_EN);
            break;
        case 2:
            enable = AC400_GET_BIT(word1, AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_INTF_2_EN);
            break;
        case 3:
            enable = AC400_GET_BIT(word1, AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_INTF_3_EN);
            break;
        default:
            return TAI_STATUS_FAILURE;
    }

    attr->value.booldata = (enable != 0);
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Set the interface enable state
 *
 * @param [in] host_interface_id The host interface identifier
 * @param [in] attr The attribute which contains the client interface enable
 *        state (bool).
 *
 * @return tai_status_t
 */
static tai_status_t ac400_set_enable(
   _In_ tai_object_id_t        host_interface_id,
   _In_ const tai_attribute_t *attr)
{
    ac400_object_id_t *hostif_obj = (ac400_object_id_t *)&host_interface_id;
    tai_object_id_t module_id = hostif_info[hostif_obj->value].module_id;
    uint16_t word1;
    uint16_t enable;

    enable = (attr->value.booldata) ? 1 : 0;

    if (ac400_mdio_read(module_id, AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_REG, &word1)) {
        return TAI_STATUS_FAILURE;
    }

    switch (hostif_info[hostif_obj->value].hostif_idx) {
        case 0:
            word1 = AC400_PUT_BIT(word1, AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_INTF_0_EN, enable);
            break;
        case 1:
            word1 = AC400_PUT_BIT(word1, AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_INTF_1_EN, enable);
            break;
        case 2:
            word1 = AC400_PUT_BIT(word1, AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_INTF_2_EN, enable);
            break;
        case 3:
            word1 = AC400_PUT_BIT(word1, AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_INTF_3_EN, enable);
            break;
        default:
            return TAI_STATUS_FAILURE;
    }

    if (ac400_mdio_write(module_id, AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_REG, word1)) {
        return TAI_STATUS_FAILURE;
    }

    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Retrieve a field from a sequential list of client interface registers.
 *
 * It is common for there to be sequential registers, one for each client
 * interface, which have the same format. This routine grabs a field from these
 * registers, given the first register address, host interface index, and lsb
 * and msb of the field.
 *
 * @param [in] host_interface_id The host interface identifier
 * @param [in,out] field Pointer to the field that is being retrieved
 * @param [in] reg_base The MDIO address of the first of the list of registers
 * @param [in] field_msb The most sigificant bit of the field being retrieved
 * @param [in] field_lsb The least sigificant bit of the field being retrieved
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_field_from_reg_list(
   _In_ tai_object_id_t     host_interface_id,
   _Inout_ uint16_t        *field,
   _In_ uint16_t            reg_base,
   _In_ int                 field_msb,
   _In_ int                 field_lsb)
{
    ac400_object_id_t *hostif_obj = (ac400_object_id_t *)&host_interface_id;
    tai_object_id_t module_id = hostif_info[hostif_obj->value].module_id;
    uint16_t word1;

    *field = 0;
    if (ac400_mdio_read(module_id, reg_base +
                        hostif_info[hostif_obj->value].hostif_idx, &word1)) {
        return TAI_STATUS_FAILURE;
    }

    *field = (word1 & GENMASK(field_msb, field_lsb)) >> field_lsb;
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Sets a field in a sequential list of client interface registers.
 *
 * It is common for there to be sequential registers, one for each client
 * interface, which have the same format. This routine modifies a field in these
 * registers, given the first register address, host interface index, lsb and
 * msb of the field, and the field value.
 *
 * @param [in] host_interface_id The host interface identifier
 * @param [in] field The value of the field that is being set
 * @param [in] reg_base The MDIO address of the first of the list of registers
 * @param [in] field_msb The most sigificant bit of the field being retrieved
 * @param [in] field_lsb The least sigificant bit of the field being retrieved
 *
 * @return tai_status_t
 */
static tai_status_t ac400_set_field_from_reg_list(
   _In_ tai_object_id_t     host_interface_id,
   _In_ uint16_t            field,
   _In_ uint16_t            reg_base,
   _In_ int                 field_msb,
   _In_ int                 field_lsb)
{
    ac400_object_id_t *hostif_obj = (ac400_object_id_t *)&host_interface_id;
    tai_object_id_t module_id = hostif_info[hostif_obj->value].module_id;
    uint16_t word1;

    if (ac400_mdio_read(module_id, reg_base +
                        hostif_info[hostif_obj->value].hostif_idx, &word1)) {
        return TAI_STATUS_FAILURE;
    }

    word1 = (word1 & ~GENMASK(field_msb, field_lsb)) | (field << field_lsb);

    if (ac400_mdio_write(module_id, reg_base +
                         hostif_info[hostif_obj->value].hostif_idx, word1)) {
        return TAI_STATUS_FAILURE;
    }

    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Retrieve the value of an attribute
 *
 * @param [in] host_interface_id The host interface ID handle
 * @param [in,out] attr A pointer to the attribute to be retrieved
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t ac400_get_host_interface_attribute(
    _In_ tai_object_id_t     host_interface_id,
    _Inout_ tai_attribute_t *attr)
{
    uint16_t field;
    tai_status_t ret;
    ac400_object_id_t *hostif_obj = (ac400_object_id_t *)&host_interface_id;

    TAI_SYSLOG_DEBUG("Retrieving host interface attribute: %d", attr->id);
    switch (attr->id) {
        case TAI_HOST_INTERFACE_ATTR_INDEX:
            attr->value.u32 = hostif_info[hostif_obj->value].hostif_idx;
            return TAI_STATUS_SUCCESS;
        case TAI_HOST_INTERFACE_ATTR_LANE_FAULTS:
            return ac400_get_lane_faults(host_interface_id, attr);
        case TAI_HOST_INTERFACE_ATTR_TX_ALIGN_STATUS:
            return ac400_get_tx_align(host_interface_id, attr);
        case TAI_HOST_INTERFACE_ATTR_RATE:
            return ac400_get_rate(host_interface_id, attr);
        case TAI_HOST_INTERFACE_ATTR_ENABLE:
            return ac400_get_enable(host_interface_id, attr);
        case TAI_HOST_INTERFACE_ATTR_FEC_DECODER:
            ret = ac400_get_field_from_reg_list(host_interface_id,
                                                &field, AC400_CLIENT_GEN_CNTL_INTF_0_REG,
                                                AC400_CLIENT_GEN_CNTL_INTF_TX_FEC_DECODER_DIS_BIT,
                                                AC400_CLIENT_GEN_CNTL_INTF_TX_FEC_DECODER_DIS_BIT);
            attr->value.booldata = !field;
            return ret;
        case TAI_HOST_INTERFACE_ATTR_FEC_ENCODER:
            ret = ac400_get_field_from_reg_list(host_interface_id,
                                                &field, AC400_CLIENT_GEN_CNTL_INTF_0_REG,
                                                AC400_CLIENT_GEN_CNTL_INTF_RX_FEC_ENCODER_DIS_BIT,
                                                AC400_CLIENT_GEN_CNTL_INTF_RX_FEC_ENCODER_DIS_BIT);
            attr->value.booldata = !field;
            return ret;
        case TAI_HOST_INTERFACE_ATTR_TX_RESET:
            ret = ac400_get_field_from_reg_list(host_interface_id,
                                                &field, AC400_CLIENT_GEN_CNTL_INTF_0_REG,
                                                AC400_CLIENT_GEN_CNTL_INTF_TX_RESET_BIT,
                                                AC400_CLIENT_GEN_CNTL_INTF_TX_RESET_BIT);
            attr->value.booldata = !!field;
            return ret;
        case TAI_HOST_INTERFACE_ATTR_RX_RESET:
            ret = ac400_get_field_from_reg_list(host_interface_id,
                                                &field, AC400_CLIENT_GEN_CNTL_INTF_0_REG,
                                                AC400_CLIENT_GEN_CNTL_INTF_RX_RESET_BIT,
                                                AC400_CLIENT_GEN_CNTL_INTF_RX_RESET_BIT);
            attr->value.booldata = !!field;
            return ret;
        case TAI_HOST_INTERFACE_ATTR_TX_DESERIAL_LF_CTLE_GAIN:
            ret = ac400_get_field_from_reg_list(host_interface_id,
                                                &field, AC400_CLIENT_HOST_TX_EQUAL_CNTL_INTF_0_REG,
                                                AC400_CLIENT_HOST_TX_EQUAL_CNTL_INTF_LF_CTLE_MSB,
                                                AC400_CLIENT_HOST_TX_EQUAL_CNTL_INTF_LF_CTLE_LSB);
            attr->value.u16 = field;
            return ret;
        case TAI_HOST_INTERFACE_ATTR_TX_DESERIAL_CTLE_GAIN:
            ret = ac400_get_field_from_reg_list(host_interface_id,
                                                &field, AC400_CLIENT_HOST_TX_EQUAL_CNTL_INTF_0_REG,
                                                AC400_CLIENT_HOST_TX_EQUAL_CNTL_INTF_CTLE_MSB,
                                                AC400_CLIENT_HOST_TX_EQUAL_CNTL_INTF_CTLE_LSB);
            attr->value.u16 = field;
            return ret;
        case TAI_HOST_INTERFACE_ATTR_TX_DESERIAL_DFE_COEFFICIENT:
            ret = ac400_get_field_from_reg_list(host_interface_id,
                                                &field, AC400_CLIENT_HOST_TX_EQUAL_CNTL_INTF_0_REG,
                                                AC400_CLIENT_HOST_TX_EQUAL_CNTL_INTF_DFE_MSB,
                                                AC400_CLIENT_HOST_TX_EQUAL_CNTL_INTF_DFE_LSB);
            attr->value.u16 = field;
            return ret;
        case TAI_HOST_INTERFACE_ATTR_RX_SERIAL_TAP0_GAIN:
            ret = ac400_get_field_from_reg_list(host_interface_id,
                                                &field, AC400_CLIENT_HOST_RX_TAP_0_CNTL_INTF_0_REG,
                                                AC400_CLIENT_HOST_RX_TAP_0_CNTL_INTF_GAIN_MSB,
                                                AC400_CLIENT_HOST_RX_TAP_0_CNTL_INTF_GAIN_LSB);
            attr->value.u16 = field;
            return ret;
        case TAI_HOST_INTERFACE_ATTR_RX_SERIAL_TAP0_DELAY:
            ret = ac400_get_field_from_reg_list(host_interface_id,
                                                &field, AC400_CLIENT_HOST_RX_TAP_0_CNTL_INTF_0_REG,
                                                AC400_CLIENT_HOST_RX_TAP_0_CNTL_INTF_DELAY_MSB,
                                                AC400_CLIENT_HOST_RX_TAP_0_CNTL_INTF_DELAY_LSB);
            attr->value.u16 = field;
            return ret;
        case TAI_HOST_INTERFACE_ATTR_RX_SERIAL_TAP1_GAIN:
            ret = ac400_get_field_from_reg_list(host_interface_id,
                                                &field, AC400_CLIENT_HOST_RX_TAP_1_CNTL_INTF_0_REG,
                                                AC400_CLIENT_HOST_RX_TAP_1_CNTL_INTF_GAIN_MSB,
                                                AC400_CLIENT_HOST_RX_TAP_1_CNTL_INTF_GAIN_LSB);
            attr->value.u16 = field;
            return ret;
        case TAI_HOST_INTERFACE_ATTR_RX_SERIAL_TAP2_GAIN:
            ret = ac400_get_field_from_reg_list(host_interface_id,
                                                &field, AC400_CLIENT_HOST_RX_TAP_2_CNTL_INTF_0_REG,
                                                AC400_CLIENT_HOST_RX_TAP_2_CNTL_INTF_GAIN_MSB,
                                                AC400_CLIENT_HOST_RX_TAP_2_CNTL_INTF_GAIN_LSB);
            attr->value.u16 = field;
            return ret;
        case TAI_HOST_INTERFACE_ATTR_RX_SERIAL_TAP2_DELAY:
            ret = ac400_get_field_from_reg_list(host_interface_id,
                                                &field, AC400_CLIENT_HOST_RX_TAP_2_CNTL_INTF_0_REG,
                                                AC400_CLIENT_HOST_RX_TAP_2_CNTL_INTF_DELAY_MSB,
                                                AC400_CLIENT_HOST_RX_TAP_2_CNTL_INTF_DELAY_LSB);
            attr->value.u16 = field;
            return ret;
    }
    return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
}

/**
 * @brief Retrieve a list of attribute values
 *
 * @param [in] host_interface_id The host interface ID handle
 * @param [in] attr_count A count of the number of elements in the attr_list
 * @param [in,out] attr_list A list of attributes to be retrieved
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t ac400_get_host_interface_attributes(
    _In_ tai_object_id_t     host_interface_id,
    _In_ uint32_t            attr_count,
    _Inout_ tai_attribute_t *attr_list)
{
    uint32_t idx;
    tai_status_t ret;

    for (idx = 0; idx < attr_count; idx++) {
        ret = ac400_get_host_interface_attribute(host_interface_id, attr_list++);
        if (ret) {
            return convert_tai_error_to_list(ret, idx);
        }
    }
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Set the value of an attribute
 *
 * @param [in] host_interface_id The host interface ID handle
 * @param [in] attr A pointer to the attribute to be set
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t ac400_set_host_interface_attribute(
   _In_ tai_object_id_t        host_interface_id,
   _In_ const tai_attribute_t *attr)
{
    uint16_t field;

    TAI_SYSLOG_DEBUG("Setting host interface attribute: %d", attr->id);
    switch (attr->id) {
        case TAI_HOST_INTERFACE_ATTR_INDEX:
            return TAI_STATUS_SUCCESS;
        case TAI_HOST_INTERFACE_ATTR_LANE_FAULTS:
        case TAI_HOST_INTERFACE_ATTR_TX_ALIGN_STATUS:
            return TAI_STATUS_INVALID_ATTRIBUTE_0;
        case TAI_HOST_INTERFACE_ATTR_RATE:
            return ac400_set_rate(host_interface_id, attr);
        case TAI_HOST_INTERFACE_ATTR_ENABLE:
            return ac400_set_enable(host_interface_id, attr);
        case TAI_HOST_INTERFACE_ATTR_FEC_DECODER:
            field = (attr->value.booldata) ? 0 : 1;
            return ac400_set_field_from_reg_list(host_interface_id, field,
                                                 AC400_CLIENT_GEN_CNTL_INTF_0_REG,
                                                 AC400_CLIENT_GEN_CNTL_INTF_TX_FEC_DECODER_DIS_BIT,
                                                 AC400_CLIENT_GEN_CNTL_INTF_TX_FEC_DECODER_DIS_BIT);
        case TAI_HOST_INTERFACE_ATTR_FEC_ENCODER:
            field = (attr->value.booldata) ? 0 : 1;
            return ac400_set_field_from_reg_list(host_interface_id, field,
                                                 AC400_CLIENT_GEN_CNTL_INTF_0_REG,
                                                 AC400_CLIENT_GEN_CNTL_INTF_RX_FEC_ENCODER_DIS_BIT,
                                                 AC400_CLIENT_GEN_CNTL_INTF_RX_FEC_ENCODER_DIS_BIT);
        case TAI_HOST_INTERFACE_ATTR_TX_RESET:
            field = (attr->value.booldata) ? 1 : 0;
            return ac400_set_field_from_reg_list(host_interface_id, field,
                                                 AC400_CLIENT_GEN_CNTL_INTF_0_REG,
                                                 AC400_CLIENT_GEN_CNTL_INTF_TX_RESET_BIT,
                                                 AC400_CLIENT_GEN_CNTL_INTF_TX_RESET_BIT);
        case TAI_HOST_INTERFACE_ATTR_RX_RESET:
            field = (attr->value.booldata) ? 1 : 0;
            return ac400_set_field_from_reg_list(host_interface_id, field,
                                                 AC400_CLIENT_GEN_CNTL_INTF_0_REG,
                                                 AC400_CLIENT_GEN_CNTL_INTF_RX_RESET_BIT,
                                                 AC400_CLIENT_GEN_CNTL_INTF_RX_RESET_BIT);
        case TAI_HOST_INTERFACE_ATTR_TX_DESERIAL_LF_CTLE_GAIN:
            return ac400_set_field_from_reg_list(host_interface_id, attr->value.u16,
                                                 AC400_CLIENT_HOST_TX_EQUAL_CNTL_INTF_0_REG,
                                                 AC400_CLIENT_HOST_TX_EQUAL_CNTL_INTF_LF_CTLE_MSB,
                                                 AC400_CLIENT_HOST_TX_EQUAL_CNTL_INTF_LF_CTLE_LSB);
        case TAI_HOST_INTERFACE_ATTR_TX_DESERIAL_CTLE_GAIN:
            return ac400_set_field_from_reg_list(host_interface_id, attr->value.u16,
                                                 AC400_CLIENT_HOST_TX_EQUAL_CNTL_INTF_0_REG,
                                                 AC400_CLIENT_HOST_TX_EQUAL_CNTL_INTF_CTLE_MSB,
                                                 AC400_CLIENT_HOST_TX_EQUAL_CNTL_INTF_CTLE_LSB);
        case TAI_HOST_INTERFACE_ATTR_TX_DESERIAL_DFE_COEFFICIENT:
            return ac400_set_field_from_reg_list(host_interface_id, attr->value.u16,
                                                 AC400_CLIENT_HOST_TX_EQUAL_CNTL_INTF_0_REG,
                                                 AC400_CLIENT_HOST_TX_EQUAL_CNTL_INTF_DFE_MSB,
                                                 AC400_CLIENT_HOST_TX_EQUAL_CNTL_INTF_DFE_LSB);
        case TAI_HOST_INTERFACE_ATTR_RX_SERIAL_TAP0_GAIN:
            return ac400_set_field_from_reg_list(host_interface_id, attr->value.u16,
                                                 AC400_CLIENT_HOST_RX_TAP_0_CNTL_INTF_0_REG,
                                                 AC400_CLIENT_HOST_RX_TAP_0_CNTL_INTF_GAIN_MSB,
                                                 AC400_CLIENT_HOST_RX_TAP_0_CNTL_INTF_GAIN_LSB);
        case TAI_HOST_INTERFACE_ATTR_RX_SERIAL_TAP0_DELAY:
            return ac400_set_field_from_reg_list(host_interface_id, attr->value.u16,
                                                 AC400_CLIENT_HOST_RX_TAP_0_CNTL_INTF_0_REG,
                                                 AC400_CLIENT_HOST_RX_TAP_0_CNTL_INTF_DELAY_MSB,
                                                 AC400_CLIENT_HOST_RX_TAP_0_CNTL_INTF_DELAY_LSB);
        case TAI_HOST_INTERFACE_ATTR_RX_SERIAL_TAP1_GAIN:
            return ac400_set_field_from_reg_list(host_interface_id, attr->value.u16,
                                                 AC400_CLIENT_HOST_RX_TAP_1_CNTL_INTF_0_REG,
                                                 AC400_CLIENT_HOST_RX_TAP_1_CNTL_INTF_GAIN_MSB,
                                                 AC400_CLIENT_HOST_RX_TAP_1_CNTL_INTF_GAIN_LSB);
        case TAI_HOST_INTERFACE_ATTR_RX_SERIAL_TAP2_GAIN:
            return ac400_set_field_from_reg_list(host_interface_id, attr->value.u16,
                                                 AC400_CLIENT_HOST_RX_TAP_2_CNTL_INTF_0_REG,
                                                 AC400_CLIENT_HOST_RX_TAP_2_CNTL_INTF_GAIN_MSB,
                                                 AC400_CLIENT_HOST_RX_TAP_2_CNTL_INTF_GAIN_LSB);
        case TAI_HOST_INTERFACE_ATTR_RX_SERIAL_TAP2_DELAY:
            return ac400_set_field_from_reg_list(host_interface_id, attr->value.u16,
                                                 AC400_CLIENT_HOST_RX_TAP_2_CNTL_INTF_0_REG,
                                                 AC400_CLIENT_HOST_RX_TAP_2_CNTL_INTF_DELAY_MSB,
                                                 AC400_CLIENT_HOST_RX_TAP_2_CNTL_INTF_DELAY_LSB);
    }
    return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
}

/**
 * @brief Set the values from a list of attributes
 *
 * @param [in] host_interface_id The host interface ID handle
 * @param [in] attr_count A count of the number of elements in the attr_list
 * @param [in] attr_list A list of attributes to be set
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t ac400_set_host_interface_attributes(
   _In_ tai_object_id_t        host_interface_id,
   _In_ uint32_t               attr_count,
   _In_ const tai_attribute_t *attr_list)
{
    uint32_t idx;
    tai_status_t ret;

    for (idx = 0; idx < attr_count; idx++) {
        ret = ac400_set_host_interface_attribute(host_interface_id, attr_list++);
        if (ret) {
            return convert_tai_error_to_list(ret, idx);
        }
    }
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Host interface initialization. After the call the capability
 *        attributes should be ready for retrieval via
 *        tai_get_host_interface_attribute().
 *
 * @param [out] host_interface_id Handle which identifies the host interface
 * @param [in] module_id Handle which identifies the module on which the host
 *        interface exists
 * @param [in] attr_count A count of the number of elements in the attr_list
 * @param [in] attr_list A list of attributes to set during initialization
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t ac400_create_host_interface(
    _Out_ tai_object_id_t *host_interface_id,
    _In_ tai_object_id_t module_id,
    _In_ uint32_t attr_count,
    _In_ const tai_attribute_t *attr_list)
{
    tai_status_t ret;
    const tai_attribute_value_t * hostif_addr;
    ac400_object_id_t *hostif_obj = (ac400_object_id_t *)host_interface_id;
    ac400_object_id_t *mod_obj = (ac400_object_id_t *)&module_id;
    int idx;

    hostif_addr = find_attribute_in_list(TAI_HOST_INTERFACE_ATTR_INDEX, attr_count, attr_list);
    if (NULL == hostif_addr) {
        TAI_SYSLOG_ERROR("The required TAI_HOST_INTERFACE_ATTR_INDEX attribute was not provided");
        return TAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }
    if (AC400_NUM_HOSTIF <= hostif_addr->u32) {
        TAI_SYSLOG_ERROR("The TAI_HOST_INTERFACE_ATTR_INDEX attribute is out of range");
        return TAI_STATUS_INVALID_PARAMETER;
    }

    idx = ((mod_obj->value-1) * AC400_NUM_HOSTIF) + hostif_addr->u32;
    hostif_obj->type = TAI_OBJECT_TYPE_HOSTIF;
    hostif_obj->value = idx;
    hostif_info[idx].initialized = true;
    hostif_info[idx].module_id   = module_id;
    hostif_info[idx].hostif_idx  = hostif_addr->u32;

    ret = ac400_set_host_interface_attributes(*host_interface_id, attr_count, attr_list);
    if (TAI_STATUS_SUCCESS != ret) {
        TAI_SYSLOG_ERROR("Error setting host interface attributes");
        return ret;
    }

    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Release all resources associated with previously created host
 *        interface
 *
 * @param [in] host_interface_id The host interface ID handle being removed
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t ac400_remove_host_interface(_In_ tai_object_id_t host_interface_id)
{
    ac400_object_id_t *hostif_obj = (ac400_object_id_t *)&host_interface_id;

    hostif_info[hostif_obj->value].initialized = false;
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief The host interface functions. This structure is retrieved via the
 *        #tai_api_query function.
 */
tai_host_interface_api_t ac400_host_interface_api = {
    .create_host_interface         = ac400_create_host_interface,
    .remove_host_interface         = ac400_remove_host_interface,
    .set_host_interface_attribute  = ac400_set_host_interface_attribute,
    .set_host_interface_attributes = ac400_set_host_interface_attributes,
    .get_host_interface_attribute  = ac400_get_host_interface_attribute,
    .get_host_interface_attributes = ac400_get_host_interface_attributes
};

