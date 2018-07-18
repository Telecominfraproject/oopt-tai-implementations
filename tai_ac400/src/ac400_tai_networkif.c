/**
 *  @file    ac400_tai_networkif.c
 *  @brief   The TAI network interface routines
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
#define __TAI_MODULE__ TAI_API_NETWORKIF

typedef struct _network_interface_info_t {
    bool            initialized;
    tai_object_id_t module_id;      /* module handle */
    int             netif_idx;      /* zero-based index on the module */
} network_interface_info_t;

static network_interface_info_t netif_info[VOYAGER_NUM_NETIF];


/**
 * @brief Retrieve the TX turn-up state
 *
 * @param [in] network_interface_id The network interface identifier
 * @param [in,out] attr The attribute into which the TX turn up state will be
 *        placed (#tai_network_interface_tx_turn_up_state_t)
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_tx_turn_up(
   _In_ tai_object_id_t     network_interface_id,
   _Inout_ tai_attribute_t *attr)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    uint16_t reg_addr = AC400_NTWK_TX_TURN_UP_STATE_LN_0_REG +
        netif_info[netif_obj->value].netif_idx;
    uint16_t word1;

    if (ac400_mdio_read(module_id, reg_addr, &word1)) {
        return TAI_STATUS_FAILURE;
    }
    attr->value.u32 = 0;
    if (word1 & (1 << AC400_NTWK_TX_TURN_UP_STATE_LN_TX_INIT_BIT))
        attr->value.u32 |= TAI_NETWORK_INTERFACE_TX_TURN_UP_PATH_INIT;
    if (word1 & (1 << AC400_NTWK_TX_TURN_UP_STATE_LN_ASIC_TX_READY_BIT))
        attr->value.u32 |= TAI_NETWORK_INTERFACE_TX_TURN_UP_DATA_PATH;
    if (word1 & (1 << AC400_NTWK_TX_TURN_UP_STATE_LN_TX_LAS_READY_OFF_BIT))
        attr->value.u32 |= TAI_NETWORK_INTERFACE_TX_TURN_UP_LASER_OFF;
    if (word1 & (1 << AC400_NTWK_TX_TURN_UP_STATE_LN_TX_LASER_READY_BIT))
        attr->value.u32 |= TAI_NETWORK_INTERFACE_TX_TURN_UP_LASER_READY;
    if (word1 & (1 << AC400_NTWK_TX_TURN_UP_STATE_LN_TX_MODULATOR_CONVERGE_BIT))
        attr->value.u32 |= TAI_NETWORK_INTERFACE_TX_TURN_UP_MODULATOR_CONV;
    if (word1 & (1 << AC400_NTWK_TX_TURN_UP_STATE_LN_TX_OUT_PWR_ADJ_BIT))
        attr->value.u32 |= TAI_NETWORK_INTERFACE_TX_TURN_UP_POWER_ADJUST;
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Retrieve the RX turn-up state
 *
 * @param [in] network_interface_id The network interface identifier
 * @param [in,out] attr The attribute into which the RX turn up state will be
 *        placed (#tai_network_interface_rx_turn_up_state_t)
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_rx_turn_up(
   _In_ tai_object_id_t     network_interface_id,
   _Inout_ tai_attribute_t *attr)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    uint16_t reg_addr = AC400_NTWK_RX_TURN_UP_STATE_LN_0_REG +
        netif_info[netif_obj->value].netif_idx;
    uint16_t word1;

    if (ac400_mdio_read(module_id, reg_addr, &word1)) {
        return TAI_STATUS_FAILURE;
    }
    attr->value.u32 = 0;
    if (word1 & (1 << AC400_NTWK_RX_TURN_UP_STATE_LN_RX_INIT_BIT))
        attr->value.u32 |= TAI_NETWORK_INTERFACE_RX_TURN_UP_PATH_INIT;
    if (word1 & (1 << AC400_NTWK_RX_TURN_UP_STATE_LN_ASIC_RX_READY_BIT))
        attr->value.u32 |= TAI_NETWORK_INTERFACE_RX_TURN_UP_DATA_PATH;
    if (word1 & (1 << AC400_NTWK_RX_TURN_UP_STATE_LN_OPTICAL_INPUT_BIT))
        attr->value.u32 |= TAI_NETWORK_INTERFACE_RX_TURN_UP_OPTICAL_SIGNAL;
    if (word1 & (1 << AC400_NTWK_RX_TURN_UP_STATE_LN_ADC_OUTPUT_BIT))
        attr->value.u32 |= TAI_NETWORK_INTERFACE_RX_TURN_UP_ADC_OUTPUT;
    if (word1 & (1 << AC400_NTWK_RX_TURN_UP_STATE_LN_DISPERSION_LOCK_BIT))
        attr->value.u32 |= TAI_NETWORK_INTERFACE_RX_TURN_UP_GOOD_DISP;
    if (word1 & (1 << AC400_NTWK_RX_TURN_UP_STATE_LN_RX_DEMOD_LOCK_BIT))
        attr->value.u32 |= TAI_NETWORK_INTERFACE_RX_TURN_UP_DEMOD_LOCK;
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Retrieve the TX alignment status
 * @param [in] network_interface_id The network interface identifier
 * @param [in,out] attr The attribute into which the TX alignment status will be
 *        placed (#tai_network_interface_tx_align_status_t)
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_tx_alignment(
   _In_ tai_object_id_t     network_interface_id,
   _Inout_ tai_attribute_t *attr)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    uint16_t reg_addr = AC400_NTWK_TX_ALGN_STAT_LN_0_REG +
        netif_info[netif_obj->value].netif_idx;
    uint16_t word1;

    if (ac400_mdio_read(module_id, reg_addr, &word1)) {
        return TAI_STATUS_FAILURE;
    }
    attr->value.u32 = 0;
    if (word1 & (1 << AC400_NTWK_TX_ALGN_STAT_LN_OUT_OF_ALGN_BIT))
        attr->value.u32 |= TAI_NETWORK_INTERFACE_TX_ALIGN_OUT;
    if (word1 & (1 << AC400_NTWK_TX_ALGN_STAT_LN_CMU_LCK_FLT_BIT))
        attr->value.u32 |= TAI_NETWORK_INTERFACE_TX_ALIGN_CMU_LOCK;
    if (word1 & (1 << AC400_NTWK_TX_ALGN_STAT_LN_REF_CLK_FLT_BIT))
        attr->value.u32 |= TAI_NETWORK_INTERFACE_TX_ALIGN_REF_CLOCK;
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Retrieve the RX alignment status
 * @param [in] network_interface_id The network interface identifier
 * @param [in,out] attr The attribute into which the RX alignment status will be
 *        placed (#tai_network_interface_rx_align_status_t)
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_rx_alignment(
   _In_ tai_object_id_t     network_interface_id,
   _Inout_ tai_attribute_t *attr)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    uint16_t reg_addr = AC400_NTWK_RX_ALGN_STAT_LN_0_REG +
        netif_info[netif_obj->value].netif_idx;
    uint16_t word1;

    if (ac400_mdio_read(module_id, reg_addr, &word1)) {
        return TAI_STATUS_FAILURE;
    }
    attr->value.u32 = 0;
    if (word1 & (1 << AC400_NTWK_RX_ALGN_STAT_LN_MODEM_SYNC_DET_FLT_BIT))
        attr->value.u32 |= TAI_NETWORK_INTERFACE_RX_ALIGN_MODEM_SYNC;
    if (word1 & (1 << AC400_NTWK_RX_ALGN_STAT_LN_MODEM_LOCK_FLT_BIT))
        attr->value.u32 |= TAI_NETWORK_INTERFACE_RX_ALIGN_MODEM_LOCK;
    if (word1 & (1 << AC400_NTWK_RX_ALGN_STAT_LN_LOSS_OF_ALGN_FLT_BIT))
        attr->value.u32 |= TAI_NETWORK_INTERFACE_RX_ALIGN_LOSS;
    if (word1 & (1 << AC400_NTWK_RX_ALGN_STAT_LN_OUT_OF_ALGN_FLT_BIT))
        attr->value.u32 |= TAI_NETWORK_INTERFACE_RX_ALIGN_OUT;
    if (word1 & (1 << AC400_NTWK_RX_ALGN_STAT_LN_TIMING_FLT_BIT))
        attr->value.u32 |= TAI_NETWORK_INTERFACE_RX_ALIGN_TIMING;
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Retrieve the current bit error rate
 * @param [in] network_interface_id The network interface identifier
 * @param [in,out] attr The attribute into which the current bit error rate will
 *        be placed (#tai_float_t)
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_ber(
   _In_ tai_object_id_t     network_interface_id,
   _Inout_ tai_attribute_t *attr)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    uint16_t reg_addr = AC400_NTWK_CUR_BER_WH_LN_0_REG +
        (netif_info[netif_obj->value].netif_idx*2);
    uint16_t word1;
    uint16_t word2;

    if (ac400_mdio_read(module_id, reg_addr++, &word1)) {
        return TAI_STATUS_FAILURE;
    }
    if (ac400_mdio_read(module_id, reg_addr, &word2)) {
        return TAI_STATUS_FAILURE;
    }
    attr->value.u32 = (word1 << 16) | word2;
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Retrieve a field from a sequential list of network interface
 *        registers.
 *
 * It is common for there to be sequential registers, one for each network
 * interface, which have the same format. This routine grabs a field from these
 * registers, given the first register address, network interface index, and lsb
 * and msb of the field.
 *
 * @param [in] network_interface_id The network interface identifier
 * @param [in,out] field Pointer to the field that is being retrieved
 * @param [in] reg_base The MDIO address of the first of the list of registers
 * @param [in] field_msb The most sigificant bit of the field being retrieved
 * @param [in] field_lsb The least sigificant bit of the field being retrieved
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_field_from_reg_list(
   _In_ tai_object_id_t     network_interface_id,
   _Inout_ uint16_t        *field,
   _In_ uint16_t            reg_base,
   _In_ int                 field_msb,
   _In_ int                 field_lsb)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    uint16_t word1;

    *field = 0;
    if (ac400_mdio_read(module_id, reg_base +
                        netif_info[netif_obj->value].netif_idx, &word1)) {
        return TAI_STATUS_FAILURE;
    }

    *field = (word1 & GENMASK(field_msb, field_lsb)) >> field_lsb;
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Sets a field in a sequential list of network interface registers.
 *
 * It is common for there to be sequential registers, one for each network
 * interface, which have the same format. This routine modifies a field in these
 * registers, given the first register address, network interface index, lsb and
 * msb of the field, and the field value.
 *
 * @param [in] network_interface_id The network interface identifier
 * @param [in] field The value of the field that is being set
 * @param [in] reg_base The MDIO address of the first of the list of registers
 * @param [in] field_msb The most sigificant bit of the field being retrieved
 * @param [in] field_lsb The least sigificant bit of the field being retrieved
 *
 * @return tai_status_t
 */
static tai_status_t ac400_set_field_from_reg_list(
   _In_ tai_object_id_t     network_interface_id,
   _In_ uint16_t            field,
   _In_ uint16_t            reg_base,
   _In_ int                 field_msb,
   _In_ int                 field_lsb)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    uint16_t word1;

    if (ac400_mdio_read(module_id, reg_base +
                        netif_info[netif_obj->value].netif_idx, &word1)) {
        return TAI_STATUS_FAILURE;
    }

    word1 = (word1 & ~GENMASK(field_msb, field_lsb)) | (field << field_lsb);

    if (ac400_mdio_write(module_id, reg_base +
                         netif_info[netif_obj->value].netif_idx, word1)) {
        return TAI_STATUS_FAILURE;
    }

    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Retrieve the FEC uncorrectable code block count
 * @param [in] network_interface_id The network interface identifier
 * @param [in,out] attr The attribute into which the FEC uncorrectable code
 *        block count will be placed (#tai_float_t)
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_fec_uncorrectable(
   _In_ tai_object_id_t     network_interface_id,
   _Inout_ tai_attribute_t *attr)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    uint16_t reg_addr = AC400_NTWK_FEC_UNCORR_CB_CNT_W1_LN_0_REG +
        (netif_info[netif_obj->value].netif_idx*4);
    uint16_t word1;
    uint16_t word2;
    uint16_t word3;
    uint16_t word4;

    if (ac400_mdio_read(module_id, reg_addr++, &word1))
        return TAI_STATUS_FAILURE;
    if (ac400_mdio_read(module_id, reg_addr++, &word2))
        return TAI_STATUS_FAILURE;
    if (ac400_mdio_read(module_id, reg_addr++, &word3))
        return TAI_STATUS_FAILURE;
    if (ac400_mdio_read(module_id, reg_addr, &word4))
        return TAI_STATUS_FAILURE;
    attr->value.u64 = ((uint64_t)word1 << 48) | ((uint64_t)word2 << 32) |
        ((uint32_t)word3 << 16) | word4;
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Retrieve the TX enable control
 * @param [in] network_interface_id The network interface identifier
 * @param [in,out] attr The attribute into which the tx enable bit will be
 *        placed (bool)
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_tx_enable(
   _In_ tai_object_id_t     network_interface_id,
   _Inout_ tai_attribute_t *attr)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    uint16_t reg_addr = AC400_IND_NTWK_LANE_TX_DIS_CNTL_REG;
    uint16_t word1;

    if (ac400_mdio_read(module_id, reg_addr, &word1)) {
        return TAI_STATUS_FAILURE;
    }
    attr->value.booldata = !(word1 & (1 << netif_info[netif_obj->value].netif_idx));
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Set the TX enable control
 * @param [in] network_interface_id The network interface identifier
 * @param [in] attr The attribute from which the tx enable bit will be set
 *        (bool)
 *
 * @return tai_status_t
 */
static tai_status_t ac400_set_tx_enable(
   _In_ tai_object_id_t        network_interface_id,
   _In_ const tai_attribute_t *attr)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    uint16_t reg_addr = AC400_IND_NTWK_LANE_TX_DIS_CNTL_REG;
    uint16_t word1;
    uint16_t ac400_enable = (attr->value.booldata) ? 0 : 1;

    if (ac400_mdio_read(module_id, reg_addr, &word1)) {
        return TAI_STATUS_FAILURE;
    }
    switch (netif_info[netif_obj->value].netif_idx) {
        case 0:
            word1 = AC400_PUT_BIT(word1, AC400_IND_NTWK_LANE_TX_DIS_CNTL_TX_DIS_LANE_0, ac400_enable);
            break;
        case 1:
            word1 = AC400_PUT_BIT(word1, AC400_IND_NTWK_LANE_TX_DIS_CNTL_TX_DIS_LANE_1, ac400_enable);
            break;
        default:
            return TAI_STATUS_FAILURE;
    }
    if (ac400_mdio_write(module_id, reg_addr, word1)) {
        return TAI_STATUS_FAILURE;
    }
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Retrieve the TX channel grid spacing
 * @param [in] network_interface_id The network interface identifier
 * @param [in,out] attr The attribute into which the TX channel grid spacing
 *        will be placed (#tai_network_interface_tx_grid_spacing_t)
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_tx_grid_spacing(
   _In_ tai_object_id_t     network_interface_id,
   _Inout_ tai_attribute_t *attr)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    uint16_t reg_addr = AC400_NTWK_TX_CHAN_CNTL_LN_0_REG +
        netif_info[netif_obj->value].netif_idx;
    uint16_t word1;
    uint16_t ac400_grid_spacing;

    if (ac400_mdio_read(module_id, reg_addr, &word1)) {
        return TAI_STATUS_FAILURE;
    }
    ac400_grid_spacing = AC400_GET_FIELD(word1, AC400_NTWK_TX_CHAN_CNTL_LN_GRID_SPACING);

    switch (ac400_grid_spacing) {
        case 0:
            attr->value.u32 = TAI_NETWORK_INTERFACE_TX_GRID_SPACING_100_GHZ;
            break;
        case 1:
            attr->value.u32 = TAI_NETWORK_INTERFACE_TX_GRID_SPACING_50_GHZ;
            break;
        case 2:
            attr->value.u32 = TAI_NETWORK_INTERFACE_TX_GRID_SPACING_33_GHZ;
            break;
        case 3:
            attr->value.u32 = TAI_NETWORK_INTERFACE_TX_GRID_SPACING_25_GHZ;
            break;
        case 4:
            attr->value.u32 = TAI_NETWORK_INTERFACE_TX_GRID_SPACING_12_5_GHZ;
            break;
        case 5:
            attr->value.u32 = TAI_NETWORK_INTERFACE_TX_GRID_SPACING_6_25_GHZ;
            break;
        default:
            attr->value.u32 = TAI_NETWORK_INTERFACE_TX_GRID_SPACING_UNKNOWN;
    }
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Set the TX channel grid spacing
 * @param [in] network_interface_id The network interface identifier
 * @param [in] attr The attribute from which the TX channel grid spacing will be
 *        set (#tai_network_interface_tx_grid_spacing_t)
 *
 * @return tai_status_t
 */
static tai_status_t ac400_set_tx_grid_spacing(
   _In_ tai_object_id_t        network_interface_id,
   _In_ const tai_attribute_t *attr)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    uint16_t reg_addr = AC400_NTWK_TX_CHAN_CNTL_LN_0_REG +
        netif_info[netif_obj->value].netif_idx;
    uint16_t word1;
    uint16_t ac400_grid_spacing;

    if (ac400_mdio_read(module_id, reg_addr, &word1)) {
        return TAI_STATUS_FAILURE;
    }
    ac400_grid_spacing = AC400_GET_FIELD(word1, AC400_NTWK_TX_CHAN_CNTL_LN_GRID_SPACING);

    switch (attr->value.u32) {
        case TAI_NETWORK_INTERFACE_TX_GRID_SPACING_100_GHZ:
            ac400_grid_spacing = 0;
            break;
        case TAI_NETWORK_INTERFACE_TX_GRID_SPACING_50_GHZ:
            ac400_grid_spacing = 1;
            break;
        case TAI_NETWORK_INTERFACE_TX_GRID_SPACING_33_GHZ:
            ac400_grid_spacing = 2;
            break;
        case TAI_NETWORK_INTERFACE_TX_GRID_SPACING_25_GHZ:
            ac400_grid_spacing = 3;
            break;
        case TAI_NETWORK_INTERFACE_TX_GRID_SPACING_12_5_GHZ:
            ac400_grid_spacing = 4;
            break;
        case TAI_NETWORK_INTERFACE_TX_GRID_SPACING_6_25_GHZ:
            ac400_grid_spacing = 5;
            break;
        default:
            return TAI_STATUS_FAILURE;
    }

    word1 = AC400_PUT_FIELD(word1, AC400_NTWK_TX_CHAN_CNTL_LN_GRID_SPACING, ac400_grid_spacing);
    if (ac400_mdio_write(module_id, reg_addr, word1)) {
        return TAI_STATUS_FAILURE;
    }
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Retrieve the TX output power
 * @param [in] network_interface_id The network interface identifier
 * @param [in,out] attr The attribute into which the tx output power will be
 *        placed (#tai_float_t)
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_output_power(
   _In_ tai_object_id_t     network_interface_id,
   _Inout_ tai_attribute_t *attr)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    uint16_t reg_addr = AC400_NTWK_TX_OUTPUT_PWR_LN_0_REG +
        netif_info[netif_obj->value].netif_idx;
    int16_t word1;

    if (ac400_mdio_read(module_id, reg_addr, (uint16_t *)&word1)) {
        return TAI_STATUS_FAILURE;
    }
    attr->value.flt = word1 / 100.0;
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Set the TX output power
 * @param [in] network_interface_id The network interface identifier
 * @param [in] attr The attribute from which the tx output power will be set
 *        (#tai_float_t)
 *
 * @return tai_status_t
 */
static tai_status_t ac400_set_output_power(
   _In_ tai_object_id_t        network_interface_id,
   _In_ const tai_attribute_t *attr)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    uint16_t reg_addr = AC400_NTWK_TX_OUTPUT_PWR_LN_0_REG +
        netif_info[netif_obj->value].netif_idx;
    int16_t word1;

    word1 = attr->value.flt * 100;
    if (ac400_mdio_write(module_id, reg_addr, word1)) {
        return TAI_STATUS_FAILURE;
    }
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Retrieve the current measured TX output power
 * @param [in] network_interface_id The network interface identifier
 * @param [in,out] attr The attribute into which the current tx output power
 *        will be placed (#tai_float_t)
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_current_output_power(
   _In_ tai_object_id_t     network_interface_id,
   _Inout_ tai_attribute_t *attr)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    uint16_t reg_addr = AC400_NTWK_TX_CUR_OUT_PWR_LN_0_REG +
        netif_info[netif_obj->value].netif_idx;
    int16_t word1;

    if (ac400_mdio_read(module_id, reg_addr, (uint16_t *)&word1)) {
        return TAI_STATUS_FAILURE;
    }
    attr->value.flt = word1 / 100.0;
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Retrieve the TX laser frequency in Hz
 * @param [in] network_interface_id The network interface identifier
 * @param [in,out] attr The attribute into which the tx laser frequency will be
 *        placed (#tai_uint64_t)
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_tx_laser_freq(
   _In_ tai_object_id_t     network_interface_id,
   _Inout_ tai_attribute_t *attr)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    uint16_t reg_addr = AC400_NTWK_TX_FREQ_1_LN_0_REG +
        netif_info[netif_obj->value].netif_idx;
    uint16_t word1;
    uint16_t word2;

    if (ac400_mdio_read(module_id, reg_addr, &word1)) {
        return TAI_STATUS_FAILURE;
    }
    reg_addr = AC400_NTWK_TX_FREQ_2_LN_0_REG + netif_info[netif_obj->value].netif_idx;
    if (ac400_mdio_read(module_id, reg_addr, &word2)) {
        return TAI_STATUS_FAILURE;
    }
    attr->value.u64 = (word1 * 1000000000000ULL) + (word2 * 50000000ULL);
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Retrieve the TX fine tune laser frequency in Hz
 * @param [in] network_interface_id The network interface identifier
 * @param [in,out] attr The attribute into which the tx fine tune laser
 *        frequency will be placed (#tai_uint64_t)
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_tx_fine_tune_laser_freq(
   _In_ tai_object_id_t     network_interface_id,
   _Inout_ tai_attribute_t *attr)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    uint16_t reg_addr = AC400_NTWK_TX_LASER_FTF_LN_0_REG +
        netif_info[netif_obj->value].netif_idx;
    uint16_t word1;

    if (ac400_mdio_read(module_id, reg_addr, &word1)) {
        return TAI_STATUS_FAILURE;
    }
    attr->value.u64 = word1 * 1000000ULL;
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Set the TX fine tune laser frequency
 * @param [in] network_interface_id The network interface identifier
 * @param [in] attr The attribute from which the tx laser fine tune frequency in
 *        Hz will be set (#tai_uint64_t)
 *
 * @return tai_status_t
 */
static tai_status_t ac400_set_tx_fine_tune_laser_freq(
   _In_ tai_object_id_t        network_interface_id,
   _In_ const tai_attribute_t *attr)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    uint16_t reg_addr = AC400_NTWK_TX_LASER_FTF_LN_0_REG +
        netif_info[netif_obj->value].netif_idx;
    uint16_t word1;

    word1 = attr->value.u64 / 1000000ULL;
    if (ac400_mdio_write(module_id, reg_addr, word1)) {
        return TAI_STATUS_FAILURE;
    }
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Retrieve the current measured RX input power
 * @param [in] network_interface_id The network interface identifier
 * @param [in,out] attr The attribute into which the current rx input power will
 *        be placed (#tai_float_t)
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_current_input_power(
   _In_ tai_object_id_t     network_interface_id,
   _Inout_ tai_attribute_t *attr)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    uint16_t reg_addr = AC400_NTWK_RX_CUR_IN_PWR_LN_0_REG +
        netif_info[netif_obj->value].netif_idx;
    int16_t word1;

    if (ac400_mdio_read(module_id, reg_addr, (uint16_t *)&word1)) {
        return TAI_STATUS_FAILURE;
    }
    attr->value.flt = word1 / 100.0;
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Retrieve the master enable
 * @param [in] network_interface_id The network interface identifier
 * @param [in,out] attr The attribute into which the master enable will be
 *        placed (bool)
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_master_enable(
   _In_ tai_object_id_t     network_interface_id,
   _Inout_ tai_attribute_t *attr)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    uint16_t reg_addr = AC400_NTWK_LANES_ENABLE_CNTL_REG;
    uint16_t word1;

    if (ac400_mdio_read(module_id, reg_addr, &word1)) {
        return TAI_STATUS_FAILURE;
    }
    attr->value.booldata = !!(word1 & (1 << netif_info[netif_obj->value].netif_idx));
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Set the master enable
 * @param [in] network_interface_id The network interface identifier
 * @param [in] attr The attribute from which the master enable will be set
 *        (bool)
 *
 * @return tai_status_t
 */
static tai_status_t ac400_set_master_enable(
   _In_ tai_object_id_t        network_interface_id,
   _In_ const tai_attribute_t *attr)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    uint16_t reg_addr = AC400_NTWK_LANES_ENABLE_CNTL_REG;
    uint16_t word1;
    uint16_t ac400_enable = (attr->value.booldata) ? 1 : 0;

    if (ac400_mdio_read(module_id, reg_addr, &word1)) {
        return TAI_STATUS_FAILURE;
    }
    switch (netif_info[netif_obj->value].netif_idx) {
        case 0:
            word1 = AC400_PUT_BIT(word1, AC400_NTWK_LANES_ENABLE_CNTL_MASTER_EN_LN_0, ac400_enable);
            break;
        case 1:
            word1 = AC400_PUT_BIT(word1, AC400_NTWK_LANES_ENABLE_CNTL_MASTER_EN_LN_1, ac400_enable);
            break;
        default:
            return TAI_STATUS_FAILURE;
    }
    if (ac400_mdio_write(module_id, reg_addr, word1)) {
        return TAI_STATUS_FAILURE;
    }
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Retrieve the modulation format
 * @param [in] network_interface_id The network interface identifier
 * @param [in,out] attr The attribute into which the modulation format will be
 *        placed (#tai_network_interface_modulation_format_t)
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_modulation_format(
   _In_ tai_object_id_t     network_interface_id,
   _Inout_ tai_attribute_t *attr)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    uint16_t reg_addr = AC400_NTWK_GEN_MODE_CNTL_LN_0_REG +
        netif_info[netif_obj->value].netif_idx;
    uint16_t word1;
    uint16_t ac400_mod;

    if (ac400_mdio_read(module_id, reg_addr, &word1)) {
        return TAI_STATUS_FAILURE;
    }
    ac400_mod = AC400_GET_FIELD(word1, AC400_NTWK_GEN_MODE_CNTL_LN_MODULATION_FORMAT);
    switch (ac400_mod) {
        case 0:
            attr->value.u32 = TAI_NETWORK_INTERFACE_MODULATION_FORMAT_16_QAM;
            break;
        case 1:
            attr->value.u32 = TAI_NETWORK_INTERFACE_MODULATION_FORMAT_QPSK;
            break;
        case 2:
            attr->value.u32 = TAI_NETWORK_INTERFACE_MODULATION_FORMAT_8_QAM;
            break;
        default:
            attr->value.u32 = TAI_NETWORK_INTERFACE_MODULATION_FORMAT_UNKNOWN;
            break;
    }
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Place module in low power or disabled in order to modify state
 *
 * Some registers in the AC400 can only be modified when the module is in the
 * low power state, or the network lane being modified is in the disabled state.
 * This routine checks the current state and modifies it if necessary.
 *
 * @param [in] network_interface_id The network interface identifier
 * @param [in,out] prev_state The attribute into which the previous module state
 *        will be stored.
 *
 * @return tai_status_t
 */
static tai_status_t move_to_low_power(
   _In_ tai_object_id_t    network_interface_id,
   _Inout_ tai_attribute_t *prev_state)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    tai_status_t ret;
    tai_attribute_t lane_mode;
    tai_attribute_t new_state;
  
    /* Get the current lane mode */
    ret = ac400_get_network_mode(module_id, &lane_mode);
    if (TAI_STATUS_SUCCESS != ret) {
        return ret;
    }
    /* Are we in coupled lane mode? */
    if (TAI_MODULE_NETWORK_MODE_COUPLED == lane_mode.value.u32) {
        ret = ac400_get_module_oper_status(module_id, prev_state);
        if (TAI_STATUS_SUCCESS != ret) {
            return ret;
        }
        if (TAI_NETWORK_INTERFACE_OPER_STATUS_LOW_POWER != prev_state->value.u32) {
            new_state.value.u32 = TAI_NETWORK_INTERFACE_OPER_STATUS_LOW_POWER;
            return ac400_set_module_oper_status(module_id, &new_state);
        }
    }
    else {
        /* If the master lane enable is set, clear it. */
        ret = ac400_get_master_enable(network_interface_id, prev_state);
        if (TAI_STATUS_SUCCESS != ret) {
            return ret;
        }
        if (prev_state->value.booldata) {
            new_state.value.booldata = false;
            return ac400_set_master_enable(network_interface_id, &new_state);
        }
    }
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Restore the saved state of a module
 *
 * Some registers in the AC400 can only be modified when the module is in the
 * low power state, or the network lane being modified is in the disabled state.
 * This routine restores the state after such a modification.
 *
 * @param [in] network_interface_id The network interface identifier
 * @param [in,out] prev_state The attribute from which the previous module state
 *        will be restored.
 *
 * @return tai_status_t
 */
static tai_status_t restore_from_low_power(
   _In_ tai_object_id_t    network_interface_id,
   _Inout_ tai_attribute_t *prev_state)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    tai_status_t ret;
    tai_attribute_t lane_mode;
  
    /* Get the current lane mode */
    ret = ac400_get_network_mode(module_id, &lane_mode);
    if (TAI_STATUS_SUCCESS != ret) {
        return ret;
    }
    /* Are we in coupled lane mode? */
    if (TAI_MODULE_NETWORK_MODE_COUPLED == lane_mode.value.u32) {
        if (TAI_NETWORK_INTERFACE_OPER_STATUS_LOW_POWER != prev_state->value.u32) {
            return ac400_set_module_oper_status(module_id, prev_state);
        }
    }
    else {
        if (prev_state->value.booldata) {
            return ac400_set_master_enable(network_interface_id, prev_state);
        }
    }
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Set the mmodulation format
 * @param [in] network_interface_id The network interface identifier
 * @param [in] attr The attribute from which the modulation format will be set
 *        (#tai_network_interface_modulation_format_t)
 *
 * @return tai_status_t
 */
static tai_status_t ac400_set_modulation_format(
   _In_ tai_object_id_t        network_interface_id,
   _In_ const tai_attribute_t *attr)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    uint16_t reg_addr = AC400_NTWK_GEN_MODE_CNTL_LN_0_REG +
        netif_info[netif_obj->value].netif_idx;
    uint16_t word1;
    uint16_t ac400_mod;
    tai_attribute_t prev_state;
    tai_status_t ret;

    ret = move_to_low_power(network_interface_id, &prev_state);
    if (TAI_STATUS_SUCCESS != ret) {
        return ret;
    }

    switch (attr->value.u32) {
        case TAI_NETWORK_INTERFACE_MODULATION_FORMAT_16_QAM:
            ac400_mod = 0;
            break;
        case TAI_NETWORK_INTERFACE_MODULATION_FORMAT_QPSK:
            ac400_mod = 1;
            break;
        case TAI_NETWORK_INTERFACE_MODULATION_FORMAT_8_QAM:
            ac400_mod = 2;
            break;
        default:
            return TAI_STATUS_FAILURE;
    }

    if (ac400_mdio_read(module_id, reg_addr, &word1)) {
        return TAI_STATUS_FAILURE;
    }
    word1 = AC400_PUT_FIELD(word1, AC400_NTWK_GEN_MODE_CNTL_LN_MODULATION_FORMAT, ac400_mod);
    if (ac400_mdio_write(module_id, reg_addr, word1)) {
        return TAI_STATUS_FAILURE;
    }

    return restore_from_low_power(network_interface_id, &prev_state);
}

/**
 * @brief Retrieve the differential encoding enable
 * @param [in] network_interface_id The network interface identifier
 * @param [in,out] attr The attribute into which the differential encoding will
 *        be placed (bool)
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_differential(
   _In_ tai_object_id_t     network_interface_id,
   _Inout_ tai_attribute_t *attr)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    uint16_t reg_addr = AC400_NTWK_GEN_MODE_CNTL_LN_0_REG +
        netif_info[netif_obj->value].netif_idx;
    uint16_t word1;

    if (ac400_mdio_read(module_id, reg_addr, &word1)) {
        return TAI_STATUS_FAILURE;
    }
    attr->value.booldata = !AC400_GET_BIT(word1, AC400_NTWK_GEN_MODE_CNTL_LN_NON_DIFF_EN);
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Set the differential encoding
 * @param [in] network_interface_id The network interface identifier
 * @param [in] attr The attribute from which the differential encoding will be
 *        set (bool)
 *
 * @return tai_status_t
 */
static tai_status_t ac400_set_differential(
   _In_ tai_object_id_t        network_interface_id,
   _In_ const tai_attribute_t *attr)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    uint16_t reg_addr = AC400_NTWK_GEN_MODE_CNTL_LN_0_REG +
        netif_info[netif_obj->value].netif_idx;
    uint16_t word1;
    tai_attribute_t prev_state;
    tai_status_t ret;

    ret = move_to_low_power(network_interface_id, &prev_state);
    if (TAI_STATUS_SUCCESS != ret) {
        return ret;
    }

    if (ac400_mdio_read(module_id, reg_addr, &word1)) {
        return TAI_STATUS_FAILURE;
    }
    word1 = AC400_PUT_BIT(word1, AC400_NTWK_GEN_MODE_CNTL_LN_NON_DIFF_EN, !attr->value.booldata);
    if (ac400_mdio_write(module_id, reg_addr, word1)) {
        return TAI_STATUS_FAILURE;
    }

    return restore_from_low_power(network_interface_id, &prev_state);
}

/**
 * @brief Get the operational status of the network interface
 * @param [in] network_interface_id The network interface identifier
 * @param [in,out] attr The attribute into which the operational status will be
 *        placed
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_netif_oper_status(
   _In_ tai_object_id_t     network_interface_id,
   _Inout_ tai_attribute_t *attr)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;

    return ac400_get_module_oper_status(module_id, attr);
}

/**
 * @brief Set the operational status of the network interface
 * @param [in] network_interface_id The network interface identifier
 * @param [in] attr The attribute from which the operational status will be set 
 *
 * @return tai_status_t
 */
static tai_status_t ac400_set_netif_oper_status(
   _In_ tai_object_id_t        network_interface_id,
   _In_ const tai_attribute_t *attr)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;

    return ac400_set_module_oper_status(module_id, attr);
}

/**
 * @brief Retrieve the FEC mode
 * @param [in] network_interface_id The network interface identifier
 * @param [in,out] attr The attribute into which the FEC mode will be placed
 *        (#tai_network_interface_fec_mode_t)
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_fec_mode(
   _In_ tai_object_id_t     network_interface_id,
   _Inout_ tai_attribute_t *attr)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    uint16_t reg_addr = AC400_NTWK_GEN_MODE_CNTL_LN_0_REG +
        netif_info[netif_obj->value].netif_idx;
    uint16_t word1;
    uint16_t ac400_fec;

    if (ac400_mdio_read(module_id, reg_addr, &word1)) {
        return TAI_STATUS_FAILURE;
    }
    ac400_fec = AC400_GET_FIELD(word1, AC400_NTWK_GEN_MODE_CNTL_LN_FEC_MODE);
    switch (ac400_fec) {
        case 0:
            attr->value.u32 = TAI_NETWORK_INTERFACE_FEC_MODE_15;
            break;
        case 1:
            attr->value.u32 = TAI_NETWORK_INTERFACE_FEC_MODE_15_NON_STD;
            break;
        case 2:
            attr->value.u32 = TAI_NETWORK_INTERFACE_FEC_MODE_25;
            break;
        default:
            attr->value.u32 = TAI_NETWORK_INTERFACE_FEC_MODE_UNKNOWN;
            break;
    }
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Set the FEC mode
 * @param [in] network_interface_id The network interface identifier
 * @param [in] attr The attribute from which the FEC mode will be set
 *        (#tai_network_interface_fec_mode_t)
 *
 * @return tai_status_t
 */
static tai_status_t ac400_set_fec_mode(
   _In_ tai_object_id_t        network_interface_id,
   _In_ const tai_attribute_t *attr)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    uint16_t reg_addr = AC400_NTWK_GEN_MODE_CNTL_LN_0_REG +
        netif_info[netif_obj->value].netif_idx;
    uint16_t word1;
    uint16_t ac400_fec;
    tai_attribute_t prev_state;
    tai_status_t ret;

    ret = move_to_low_power(network_interface_id, &prev_state);
    if (TAI_STATUS_SUCCESS != ret) {
        return ret;
    }

    switch (attr->value.u32) {
        case TAI_NETWORK_INTERFACE_FEC_MODE_15:
            ac400_fec = 0;
            break;
        case TAI_NETWORK_INTERFACE_FEC_MODE_15_NON_STD:
            ac400_fec = 1;
            break;
        case TAI_NETWORK_INTERFACE_FEC_MODE_25:
            ac400_fec = 2;
            break;
        default:
            return TAI_STATUS_FAILURE;
    }
    if (ac400_mdio_read(module_id, reg_addr, &word1)) {
        return TAI_STATUS_FAILURE;
    }
    word1 = AC400_PUT_FIELD(word1, AC400_NTWK_GEN_MODE_CNTL_LN_FEC_MODE, ac400_fec);
    if (ac400_mdio_write(module_id, reg_addr, word1)) {
        return TAI_STATUS_FAILURE;
    }

    return restore_from_low_power(network_interface_id, &prev_state);
}

/**
 * @brief Retrieve a laser frequency from the AC400
 *
 * The laser frequency is stored in 4 consecutive MDIO registers. The first two
 * contain the frequency in THz and the next two contain the frequency in 50Mhz.
 *
 * @param [in] network_interface_id The AC400 network interface identifier
 * @param [in,out] attr The attribute into which the frequency (in Hz) is placed
 * @param [in] reg_addr The first AC400 register address
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_laser_freq(
   _In_    tai_object_id_t  network_interface_id,
   _Inout_ tai_attribute_t *attr,
   _In_    uint16_t         reg_addr)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    uint16_t word1, word2;

    /* Get the THz portion */
    if (ac400_mdio_read(module_id, reg_addr++, &word1)) {
        return TAI_STATUS_FAILURE;
    }
    if (ac400_mdio_read(module_id, reg_addr++, &word2)) {
        return TAI_STATUS_FAILURE;
    }
    attr->value.u64 = (word1*256 + word2) * 1000000000000LL;

    /* Get the 50MHz portion */
    if (ac400_mdio_read(module_id, reg_addr++, &word1)) {
        return TAI_STATUS_FAILURE;
    }
    if (ac400_mdio_read(module_id, reg_addr++, &word2)) {
        return TAI_STATUS_FAILURE;
    }
    attr->value.u64 += (word1*256 + word2) * 50000000LL;

    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Retrieve the laser fine tune frequency range
 *
 * @param [in] network_interface_id The AC400 network interface identifier
 * @param [in,out] attr The attribute into which the frequency range (in Hz) is
 *        placed
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_laser_freq_range(
   _In_ tai_object_id_t     network_interface_id,
   _Inout_ tai_attribute_t *attr)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    uint16_t word1, word2;

    /* Get the range in MHz */
    if (ac400_mdio_read(module_id, AC400_TX_LASER_FTF_RANGE_MSB_REG, &word1)) {
        return TAI_STATUS_FAILURE;
    }
    if (ac400_mdio_read(module_id, AC400_TX_LASER_FTF_RANGE_LSB_REG, &word2)) {
        return TAI_STATUS_FAILURE;
    }
    attr->value.u64 = (word1*256 + word2) * 1000000LL;

    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Retrieve the laser tuing grid spacing capabilities
 *
 * @param [in] network_interface_id The AC400 network interface identifier
 * @param [in,out] attr The attribute into which the supported grid spacing
 *             capabilities (a bitmap) are placed
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_grid_spacing(
   _In_ tai_object_id_t     network_interface_id,
   _Inout_ tai_attribute_t *attr)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    uint16_t word;

    if (ac400_mdio_read(module_id, AC400_LASER_TUNE_CAP_MSB_REG, &word)) {
        return TAI_STATUS_FAILURE;
    }

    attr->value.u32 = 0;

    if (AC400_GET_BIT(word, AC400_LASER_TUNE_CAP_MSB_GRID_SPACING_6P25_GHZ))
        attr->value.u32 |= TAI_NETWORK_INTERFACE_LASER_GRID_SPACING_6_25_GHZ;
    if (AC400_GET_BIT(word, AC400_LASER_TUNE_CAP_MSB_GRID_SPACING_12P5_GHZ))
        attr->value.u32 |= TAI_NETWORK_INTERFACE_LASER_GRID_SPACING_12_5_GHZ;
    if (AC400_GET_BIT(word, AC400_LASER_TUNE_CAP_MSB_GRID_SPACING_25_GHZ))
        attr->value.u32 |= TAI_NETWORK_INTERFACE_LASER_GRID_SPACING_25_GHZ;
    if (AC400_GET_BIT(word, AC400_LASER_TUNE_CAP_MSB_GRID_SPACING_33_GHZ))
        attr->value.u32 |= TAI_NETWORK_INTERFACE_LASER_GRID_SPACING_33_GHZ;
    if (AC400_GET_BIT(word, AC400_LASER_TUNE_CAP_MSB_GRID_SPACING_50_GHZ))
        attr->value.u32 |= TAI_NETWORK_INTERFACE_LASER_GRID_SPACING_50_GHZ;
    if (AC400_GET_BIT(word, AC400_LASER_TUNE_CAP_MSB_GRID_SPACING_100_GHZ))
        attr->value.u32 |= TAI_NETWORK_INTERFACE_LASER_GRID_SPACING_100_GHZ;

    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Retrieve the maximum number of channels which can be selected
 *
 * @param [in] network_interface_id The AC400 network interface identifier
 * @param [in,out] attr The attribute into which the number of channels is
 *        placed
 *
 * @return tai_status_t
 */
static tai_status_t ac400_get_max_laser_channels(
   _In_ tai_object_id_t     network_interface_id,
   _Inout_ tai_attribute_t *attr)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;
    tai_object_id_t module_id = netif_info[netif_obj->value].module_id;
    uint16_t word1, word2;

    if (ac400_mdio_read(module_id, AC400_LASER_TUNE_CAP_MSB_REG, &word1)) {
        return TAI_STATUS_FAILURE;
    }
    if (ac400_mdio_read(module_id, AC400_LASER_TUNE_CAP_LSB_REG, &word2)) {
        return TAI_STATUS_FAILURE;
    }

    attr->value.u32 = AC400_GET_FIELD(word1, AC400_LASER_TUNE_CAP_MSB_MAX_CHANS_BITS_9_TO_8)*256 + word2;

    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Retrieve the value of an attribute
 *
 * @param [in] network_interface_id The network interface ID handle
 * @param [in,out] attr A pointer to the attribute to be retrieved
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t ac400_get_network_interface_attribute(
    _In_ tai_object_id_t     network_interface_id,
    _Inout_ tai_attribute_t *attr)
{
    uint16_t field;
    tai_status_t ret;
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;

    TAI_SYSLOG_DEBUG("Retrieving network interface attribute: %d", attr->id);
    switch (attr->id) {
        case TAI_NETWORK_INTERFACE_ATTR_INDEX:
            attr->value.u32 = netif_info[netif_obj->value].netif_idx;
            return TAI_STATUS_SUCCESS;
        case TAI_NETWORK_INTERFACE_ATTR_TX_TURN_UP_STATE:
            return ac400_get_tx_turn_up(network_interface_id, attr);
        case TAI_NETWORK_INTERFACE_ATTR_RX_TURN_UP_STATE:
            return ac400_get_rx_turn_up(network_interface_id, attr);
        case TAI_NETWORK_INTERFACE_ATTR_TX_ALIGN_STATUS:
            return ac400_get_tx_alignment(network_interface_id, attr);
        case TAI_NETWORK_INTERFACE_ATTR_RX_ALIGN_STATUS:
            return ac400_get_rx_alignment(network_interface_id, attr);
        case TAI_NETWORK_INTERFACE_ATTR_CURRENT_BER:
            return ac400_get_ber(network_interface_id, attr);
        case TAI_NETWORK_INTERFACE_ATTR_CURRENT_BER_PERIOD:
            attr->value.u32 = 10000;
            return TAI_STATUS_SUCCESS;
        case TAI_NETWORK_INTERFACE_ATTR_CLEAR_FEC_COUNTERS:
            ret = ac400_get_field_from_reg_list(network_interface_id, &field,
                                                AC400_NTWK_FEC_ACCUM_CNTS_CNTL_LN_0_REG,
                                                AC400_NTWK_FEC_ACCUM_CNTS_CNTL_LN_RST_ALL_ACCUM_COUNTS_BIT,
                                                AC400_NTWK_FEC_ACCUM_CNTS_CNTL_LN_RST_ALL_ACCUM_COUNTS_BIT);
            attr->value.booldata = !!field;
            return ret;
        case TAI_NETWORK_INTERFACE_ATTR_FEC_UNCORRECTABLE:
            return ac400_get_fec_uncorrectable(network_interface_id, attr);
        case TAI_NETWORK_INTERFACE_ATTR_TX_ENABLE:
            return ac400_get_tx_enable(network_interface_id, attr);
        case TAI_NETWORK_INTERFACE_ATTR_TX_GRID_SPACING:
            return ac400_get_tx_grid_spacing(network_interface_id, attr);
        case TAI_NETWORK_INTERFACE_ATTR_TX_CHANNEL:
            ret = ac400_get_field_from_reg_list(network_interface_id, &field,
                                                AC400_NTWK_TX_CHAN_CNTL_LN_0_REG,
                                                AC400_NTWK_TX_CHAN_CNTL_LN_CHAN_NUM_MSB,
                                                AC400_NTWK_TX_CHAN_CNTL_LN_CHAN_NUM_LSB);
            attr->value.u16 = field;
            return ret;
        case TAI_NETWORK_INTERFACE_ATTR_OUTPUT_POWER:
            return ac400_get_output_power(network_interface_id, attr);
        case TAI_NETWORK_INTERFACE_ATTR_CURRENT_OUTPUT_POWER:
            return ac400_get_current_output_power(network_interface_id, attr);
        case TAI_NETWORK_INTERFACE_ATTR_TX_LASER_FREQ:
            return ac400_get_tx_laser_freq(network_interface_id, attr);
        case TAI_NETWORK_INTERFACE_ATTR_TX_FINE_TUNE_LASER_FREQ:
            return ac400_get_tx_fine_tune_laser_freq(network_interface_id, attr);
        case TAI_NETWORK_INTERFACE_ATTR_CURRENT_INPUT_POWER:
            return ac400_get_current_input_power(network_interface_id, attr);
        case TAI_NETWORK_INTERFACE_ATTR_MASTER_ENABLE:
            return ac400_get_master_enable(network_interface_id, attr);
        case TAI_NETWORK_INTERFACE_ATTR_MODULATION_FORMAT:
            return ac400_get_modulation_format(network_interface_id, attr);
        case TAI_NETWORK_INTERFACE_ATTR_DIFFERENTIAL_ENCODING:
            return ac400_get_differential(network_interface_id, attr);
        case TAI_NETWORK_INTERFACE_ATTR_OPER_STATUS:
            return ac400_get_netif_oper_status(network_interface_id, attr);
        case TAI_NETWORK_INTERFACE_ATTR_MIN_LASER_FREQ:
            return ac400_get_laser_freq(network_interface_id, attr, AC400_TX_RX_MIN_LASER_FREQ_1_MSB_REG);
        case TAI_NETWORK_INTERFACE_ATTR_MAX_LASER_FREQ:
            return ac400_get_laser_freq(network_interface_id, attr, AC400_TX_RX_MAX_LASER_FREQ_1_MSB_REG);
        case TAI_NETWORK_INTERFACE_ATTR_FINE_TUNE_LASER_FREQ:
            return ac400_get_laser_freq_range(network_interface_id, attr);
        case TAI_NETWORK_INTERFACE_ATTR_LASER_GRID_SUPPORT:
            return ac400_get_grid_spacing(network_interface_id, attr);
        case TAI_NETWORK_INTERFACE_ATTR_MAX_LASER_CHANNELS:
            return ac400_get_max_laser_channels(network_interface_id, attr);
        case TAI_NETWORK_INTERFACE_ATTR_FEC_MODE:
            return ac400_get_fec_mode(network_interface_id, attr);
        case TAI_NETWORK_INTERFACE_ATTR_TX_RESET:
            ret = ac400_get_field_from_reg_list(network_interface_id, &field,
                                                AC400_NTWK_TX_CNTL_LN_0_REG,
                                                AC400_NTWK_TX_CNTL_LN_TX_RESET_BIT,
                                                AC400_NTWK_TX_CNTL_LN_TX_RESET_BIT);
            attr->value.booldata = !!field;
            return ret;
        case TAI_NETWORK_INTERFACE_ATTR_TX_FIFO_RESET:
            ret = ac400_get_field_from_reg_list(network_interface_id, &field,
                                                AC400_NTWK_TX_CNTL_LN_0_REG,
                                                AC400_NTWK_TX_CNTL_LN_TX_FIFO_RST_BIT,
                                                AC400_NTWK_TX_CNTL_LN_TX_FIFO_RST_BIT);
            attr->value.booldata = !!field;
            return ret;
        case TAI_NETWORK_INTERFACE_ATTR_RX_RESET:
            ret = ac400_get_field_from_reg_list(network_interface_id, &field,
                                                AC400_NTWK_RX_CNTL_LN_0_REG,
                                                AC400_NTWK_RX_CNTL_LN_RX_RESET_BIT,
                                                AC400_NTWK_RX_CNTL_LN_RX_RESET_BIT);
            attr->value.booldata = !!field;
            return ret;
        case TAI_NETWORK_INTERFACE_ATTR_RX_FIFO_RESET:
            ret = ac400_get_field_from_reg_list(network_interface_id, &field,
                                                AC400_NTWK_RX_CNTL_LN_0_REG,
                                                AC400_NTWK_RX_CNTL_LN_RX_FIFO_RST_BIT,
                                                AC400_NTWK_RX_CNTL_LN_RX_FIFO_RST_BIT);
            attr->value.booldata = !!field;
            return ret;
    }
    return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
}

/**
 * @brief Retrieve a list of attribute values
 *
 * @param [in] network_interface_id The network interface ID handle
 * @param [in] attr_count A count of the number of elements in the attr_list
 * @param [in,out] attr_list A list of attributes to be retrieved
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t ac400_get_network_interface_attributes(
    _In_ tai_object_id_t     network_interface_id,
    _In_ uint32_t            attr_count,
    _Inout_ tai_attribute_t *attr_list)
{
    uint32_t idx;
    tai_status_t ret;

    for (idx = 0; idx < attr_count; idx++) {
        ret = ac400_get_network_interface_attribute(network_interface_id, attr_list++);
        if (ret) {
            return convert_tai_error_to_list(ret, idx);
        }
    }
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Set the value of an attribute
 *
 * @param [in] network_interface_id The network interface ID handle
 * @param [in] attr A pointer to the attribute to be set
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t ac400_set_network_interface_attribute(
   _In_ tai_object_id_t        network_interface_id,
   _In_ const tai_attribute_t *attr)
{
    uint16_t field;

    TAI_SYSLOG_DEBUG("Setting network interface attribute: %d", attr->id);
    switch (attr->id) {
        case TAI_NETWORK_INTERFACE_ATTR_INDEX:
            return TAI_STATUS_SUCCESS;
        case TAI_NETWORK_INTERFACE_ATTR_TX_TURN_UP_STATE:
        case TAI_NETWORK_INTERFACE_ATTR_RX_TURN_UP_STATE:
        case TAI_NETWORK_INTERFACE_ATTR_TX_ALIGN_STATUS:
        case TAI_NETWORK_INTERFACE_ATTR_RX_ALIGN_STATUS:
        case TAI_NETWORK_INTERFACE_ATTR_CURRENT_BER:
        case TAI_NETWORK_INTERFACE_ATTR_CURRENT_BER_PERIOD:
        case TAI_NETWORK_INTERFACE_ATTR_FEC_UNCORRECTABLE:
        case TAI_NETWORK_INTERFACE_ATTR_CURRENT_OUTPUT_POWER:
        case TAI_NETWORK_INTERFACE_ATTR_TX_LASER_FREQ:
        case TAI_NETWORK_INTERFACE_ATTR_CURRENT_INPUT_POWER:
        case TAI_NETWORK_INTERFACE_ATTR_MIN_LASER_FREQ:
        case TAI_NETWORK_INTERFACE_ATTR_MAX_LASER_FREQ:
        case TAI_NETWORK_INTERFACE_ATTR_FINE_TUNE_LASER_FREQ:
        case TAI_NETWORK_INTERFACE_ATTR_LASER_GRID_SUPPORT:
        case TAI_NETWORK_INTERFACE_ATTR_MAX_LASER_CHANNELS:
            return TAI_STATUS_INVALID_ATTRIBUTE_0;
        case TAI_NETWORK_INTERFACE_ATTR_CLEAR_FEC_COUNTERS:
            field = (attr->value.booldata) ? 0 : 1;
            return ac400_set_field_from_reg_list(network_interface_id, field,
                                                 AC400_NTWK_FEC_ACCUM_CNTS_CNTL_LN_0_REG,
                                                 AC400_NTWK_FEC_ACCUM_CNTS_CNTL_LN_RST_ALL_ACCUM_COUNTS_BIT,
                                                 AC400_NTWK_FEC_ACCUM_CNTS_CNTL_LN_RST_ALL_ACCUM_COUNTS_BIT);
        case TAI_NETWORK_INTERFACE_ATTR_TX_ENABLE:
            return ac400_set_tx_enable(network_interface_id, attr);
        case TAI_NETWORK_INTERFACE_ATTR_TX_GRID_SPACING:
            return ac400_set_tx_grid_spacing(network_interface_id, attr);
        case TAI_NETWORK_INTERFACE_ATTR_TX_CHANNEL:
            field = attr->value.u16;
            return ac400_set_field_from_reg_list(network_interface_id, field,
                                                 AC400_NTWK_TX_CHAN_CNTL_LN_0_REG,
                                                 AC400_NTWK_TX_CHAN_CNTL_LN_CHAN_NUM_MSB,
                                                 AC400_NTWK_TX_CHAN_CNTL_LN_CHAN_NUM_LSB);
        case TAI_NETWORK_INTERFACE_ATTR_OUTPUT_POWER:
            return ac400_set_output_power(network_interface_id, attr);
        case TAI_NETWORK_INTERFACE_ATTR_TX_FINE_TUNE_LASER_FREQ:
            return ac400_set_tx_fine_tune_laser_freq(network_interface_id, attr);
        case TAI_NETWORK_INTERFACE_ATTR_MASTER_ENABLE:
            return ac400_set_master_enable(network_interface_id, attr);
        case TAI_NETWORK_INTERFACE_ATTR_MODULATION_FORMAT:
            return ac400_set_modulation_format(network_interface_id, attr);
        case TAI_NETWORK_INTERFACE_ATTR_DIFFERENTIAL_ENCODING:
            return ac400_set_differential(network_interface_id, attr);
        case TAI_NETWORK_INTERFACE_ATTR_OPER_STATUS:
            return ac400_set_netif_oper_status(network_interface_id, attr);
        case TAI_NETWORK_INTERFACE_ATTR_FEC_MODE:
            return ac400_set_fec_mode(network_interface_id, attr);
        case TAI_NETWORK_INTERFACE_ATTR_TX_RESET:
            field = attr->value.booldata;
            return ac400_set_field_from_reg_list(network_interface_id, field,
                                                 AC400_NTWK_TX_CNTL_LN_0_REG,
                                                 AC400_NTWK_TX_CNTL_LN_TX_RESET_BIT,
                                                 AC400_NTWK_TX_CNTL_LN_TX_RESET_BIT);
        case TAI_NETWORK_INTERFACE_ATTR_TX_FIFO_RESET:
            field = attr->value.booldata;
            return ac400_set_field_from_reg_list(network_interface_id, field,
                                                 AC400_NTWK_TX_CNTL_LN_0_REG,
                                                 AC400_NTWK_TX_CNTL_LN_TX_FIFO_RST_BIT,
                                                 AC400_NTWK_TX_CNTL_LN_TX_FIFO_RST_BIT);
        case TAI_NETWORK_INTERFACE_ATTR_RX_RESET:
            field = attr->value.booldata;
            return ac400_set_field_from_reg_list(network_interface_id, field,
                                                 AC400_NTWK_RX_CNTL_LN_0_REG,
                                                 AC400_NTWK_RX_CNTL_LN_RX_RESET_BIT,
                                                 AC400_NTWK_RX_CNTL_LN_RX_RESET_BIT);
        case TAI_NETWORK_INTERFACE_ATTR_RX_FIFO_RESET:
            field = attr->value.booldata;
            return ac400_set_field_from_reg_list(network_interface_id, field,
                                                 AC400_NTWK_RX_CNTL_LN_0_REG,
                                                 AC400_NTWK_RX_CNTL_LN_RX_FIFO_RST_BIT,
                                                 AC400_NTWK_RX_CNTL_LN_RX_FIFO_RST_BIT);
    }
    return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
}

/**
 * @brief Set the values from a list of attributes
 *
 * @param [in] network_interface_id The network interface ID handle
 * @param [in] attr_count A count of the number of elements in the attr_list
 * @param [in] attr_list A list of attributes to be set
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t ac400_set_network_interface_attributes(
   _In_ tai_object_id_t        network_interface_id,
   _In_ uint32_t               attr_count,
   _In_ const tai_attribute_t *attr_list)
{
    uint32_t idx;
    tai_status_t ret;

    for (idx = 0; idx < attr_count; idx++) {
        ret = ac400_set_network_interface_attribute(network_interface_id, attr_list++);
        if (ret) {
            return convert_tai_error_to_list(ret, idx);
        }
    }
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Network interface initialization. After the call the capability
 *        attributes should be ready for retrieval via
 *        tai_get_network_interface_attribute().
 *
 * @param [out] network_interface_id Handle which identifies the network
 *        interface
 * @param [in] module_id Module id on which the network interface exists
 * @param [in] attr_count A count of the number of elements in the attr_list
 * @param [in] attr_list A list of attributes to set during initialization
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t ac400_create_network_interface(
    _Out_ tai_object_id_t *network_interface_id,
    _In_ tai_object_id_t module_id,
    _In_ uint32_t attr_count,
    _In_ const tai_attribute_t *attr_list)
{
    tai_status_t ret;
    const tai_attribute_value_t * netif_addr;
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)network_interface_id;
    ac400_object_id_t *mod_obj = (ac400_object_id_t *)&module_id;
    int idx;

    netif_addr = find_attribute_in_list(TAI_NETWORK_INTERFACE_ATTR_INDEX, attr_count, attr_list);
    if (NULL == netif_addr) {
        TAI_SYSLOG_ERROR("The required TAI_NETWORK_INTERFACE_ATTR_INDEX attribute was not provided");
        return TAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    if (AC400_NUM_NETIF <= netif_addr->u32) {
        TAI_SYSLOG_ERROR("The TAI_NETWORK_INTERFACE_ATTR_INDEX attribute is out of range");
        return TAI_STATUS_INVALID_PARAMETER;
    }

    idx = ((mod_obj->value-1) * AC400_NUM_NETIF) + netif_addr->u32;
    netif_obj->type = TAI_OBJECT_TYPE_NETWORKIF;
    netif_obj->value = idx;
    netif_info[idx].initialized = true;
    netif_info[idx].module_id   = module_id;
    netif_info[idx].netif_idx   = netif_addr->u32;

    ret = ac400_set_network_interface_attributes(*network_interface_id, attr_count, attr_list);
    if (TAI_STATUS_SUCCESS != ret) {
        TAI_SYSLOG_ERROR("Error setting network interface attributes");
        return ret;
    }

    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Release all resources associated with previously created network
 *        interface
 *
 * @param [in] network_interface_id The network interface ID handle being
 *        removed
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t ac400_remove_network_interface(_In_ tai_object_id_t network_interface_id)
{
    ac400_object_id_t *netif_obj = (ac400_object_id_t *)&network_interface_id;

    netif_info[netif_obj->value].initialized = false;
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief The network interface functions. This structure is retrieved via the
 *        #tai_api_query function.
 */
tai_network_interface_api_t ac400_network_interface_api = {
    .create_network_interface         = ac400_create_network_interface,
    .remove_network_interface         = ac400_remove_network_interface,
    .set_network_interface_attribute  = ac400_set_network_interface_attribute,
    .set_network_interface_attributes = ac400_set_network_interface_attributes,
    .get_network_interface_attribute  = ac400_get_network_interface_attribute,
    .get_network_interface_attributes = ac400_get_network_interface_attributes
};

