/**
 * @file    ac400.h
 * @brief   Acacia AC400 register and field definitions
 * @author  Scott Emery <scotte@cumulusnetworks.com>
 *
 * @copyright Copyright (C) 2018 Cumulus Networks, Inc. All rights reserved
 *
 * @remark  This source code is licensed under the BSD 3-Clause license found in
 *          the LICENSE file in the root directory of this source tree.
 */

#ifndef AC400_H__
#define AC400_H__

#define GENMASK(h, l) \
        (((~0UL) << (l)) & (~0UL >> (sizeof(long)*8 - 1 - (h))))

/**
 *  @brief Retrieve the value of a field in a register. This doesn't actually
 *         access the hardware, it merely isolates and shifts a value to extract
 *         a field.
 *  @param [in] _reg The register value which contains the _field
 *  @param [in] _field The name of the field, from the constants, below, without
 *         the trailing _LSB or _MSB, e.g. AC400_EXT_MOD_ID_LANE_RATIO_TYPE.
 *  @return The value of the field
 */
#define AC400_GET_FIELD(_reg, _field) \
        (((_reg) & GENMASK(_field##_MSB, _field##_LSB)) >> _field##_LSB)

/**
 *  @brief Set the value of a field in a register. This doesn't actually access
 *         the hardware, it merely isolates and shifts a field within a value to
 *         place that field within the value.
 *  @param [in] _reg The register value which contains the _field
 *  @param [in] _field The name of the field, from the constants, below, without
 *         the trailing _LSB or _MSB, e.g. AC400_EXT_MOD_ID_LANE_RATIO_TYPE.
 *  @param [in] _val The value of the field to place within the register.
 *  @return The new register value with the modified field value
 */
#define AC400_PUT_FIELD(_reg, _field, _val) \
        (((_reg) & ~GENMASK(_field##_MSB, _field##_LSB)) | ((_val) << _field##_LSB))

/**
 *  @brief Get the value of a bit in a register. Very similar to
 *         #AC400_GET_FIELD except it operates on only one bit.
 *  @param [in] _reg The register value which contains the _bit
 *  @param [in] _bit The name of the bit field, from the constants, below,
 *         without the trailing _BIT, e.g. AC400_EXT_MOD_ID_CLE_PRESENCE.
 *  @return The of the bit field
 */
#define AC400_GET_BIT(_reg, _bit) \
        (((_reg) >> _bit##_BIT) & 1)

/**
 *  @brief Put the value of a bit in a register. Very similar to
 *         #AC400_PUT_FIELD except it operates on only one bit.
 *  @param [in] _reg The register value which contains the _bit
 *  @param [in] _bit The name of the bit field, from the constants, below,
 *         without the trailing _BIT, e.g. AC400_EXT_MOD_ID_CLE_PRESENCE.
 *  @param [in] _val The value of the bit. Any non-zero value is 1.
 *  @return The new register value with the modified bit field value
 */
#define AC400_PUT_BIT(_reg, _bit, _val) \
        (((_reg) & ~(1 << _bit##_BIT)) | ((!!(_val)) << _bit##_BIT))


/*******************************************************************************

                          AC400 Regsiter Definitions

    These register definitions are taken from the Acacia 400G Telecom Infra
    Project Contribution, v1.0, October 27, 2016, Doc #100-0144-00.

*******************************************************************************/


/*------------------------------------------------------------------------------

                          NVR 1. Basic ID Registers
                               0x8000 - 0x807F

------------------------------------------------------------------------------*/

#define AC400_VEND_NAME_BYTE_1_REG                                      0x8021
#define AC400_VEND_NAME_BYTE_2_REG                                      0x8022
#define AC400_VEND_NAME_BYTE_3_REG                                      0x8023
#define AC400_VEND_NAME_BYTE_4_REG                                      0x8024
#define AC400_VEND_NAME_BYTE_5_REG                                      0x8025
#define AC400_VEND_NAME_BYTE_6_REG                                      0x8026
#define AC400_VEND_NAME_BYTE_7_REG                                      0x8027
#define AC400_VEND_NAME_BYTE_8_REG                                      0x8028
#define AC400_VEND_NAME_BYTE_9_REG                                      0x8029
#define AC400_VEND_NAME_BYTE_10_REG                                     0x802A
#define AC400_VEND_NAME_BYTE_11_REG                                     0x802B
#define AC400_VEND_NAME_BYTE_12_REG                                     0x802C
#define AC400_VEND_NAME_BYTE_13_REG                                     0x802D
#define AC400_VEND_NAME_BYTE_14_REG                                     0x802E
#define AC400_VEND_NAME_BYTE_15_REG                                     0x802F
#define AC400_VEND_NAME_BYTE_16_REG                                     0x8030

#define AC400_VEND_PN_BYTE_1_REG                                        0x8034
#define AC400_VEND_PN_BYTE_2_REG                                        0x8035
#define AC400_VEND_PN_BYTE_3_REG                                        0x8036
#define AC400_VEND_PN_BYTE_4_REG                                        0x8037
#define AC400_VEND_PN_BYTE_5_REG                                        0x8038
#define AC400_VEND_PN_BYTE_6_REG                                        0x8039
#define AC400_VEND_PN_BYTE_7_REG                                        0x803A
#define AC400_VEND_PN_BYTE_8_REG                                        0x803B
#define AC400_VEND_PN_BYTE_9_REG                                        0x803C
#define AC400_VEND_PN_BYTE_10_REG                                       0x803D
#define AC400_VEND_PN_BYTE_11_REG                                       0x803E
#define AC400_VEND_PN_BYTE_12_REG                                       0x803F
#define AC400_VEND_PN_BYTE_13_REG                                       0x8040
#define AC400_VEND_PN_BYTE_14_REG                                       0x8041
#define AC400_VEND_PN_BYTE_15_REG                                       0x8042
#define AC400_VEND_PN_BYTE_16_REG                                       0x8043

#define AC400_VEND_SN_BYTE_1_REG                                        0x8044
#define AC400_VEND_SN_BYTE_2_REG                                        0x8045
#define AC400_VEND_SN_BYTE_3_REG                                        0x8046
#define AC400_VEND_SN_BYTE_4_REG                                        0x8047
#define AC400_VEND_SN_BYTE_5_REG                                        0x8048
#define AC400_VEND_SN_BYTE_6_REG                                        0x8049
#define AC400_VEND_SN_BYTE_7_REG                                        0x804A
#define AC400_VEND_SN_BYTE_8_REG                                        0x804B
#define AC400_VEND_SN_BYTE_9_REG                                        0x804C
#define AC400_VEND_SN_BYTE_10_REG                                       0x804D
#define AC400_VEND_SN_BYTE_11_REG                                       0x804E
#define AC400_VEND_SN_BYTE_12_REG                                       0x804F
#define AC400_VEND_SN_BYTE_13_REG                                       0x8050
#define AC400_VEND_SN_BYTE_14_REG                                       0x8051
#define AC400_VEND_SN_BYTE_15_REG                                       0x8052
#define AC400_VEND_SN_BYTE_16_REG                                       0x8053

#define AC400_FIRM_A_VER_NUM_X_REG                                      0x806C
#define AC400_FIRM_A_VER_NUM_Y_REG                                      0x806D

#define AC400_FIRM_B_VER_NUM_X_REG                                      0x807B
#define AC400_FIRM_B_VER_NUM_Y_REG                                      0x807C


/*------------------------------------------------------------------------------

                   NVR 4. MSA-100GLH Extended ID Registers
                               0x8180 - 0x81FF

------------------------------------------------------------------------------*/

#define AC400_TX_RX_MIN_LASER_FREQ_1_MSB_REG                            0x818A
#define AC400_TX_RX_MIN_LASER_FREQ_1_LSB_REG                            0x818B

#define AC400_TX_RX_MIN_LASER_FREQ_2_MSB_REG                            0x818C
#define AC400_TX_RX_MIN_LASER_FREQ_2_LSB_REG                            0x818D

#define AC400_TX_RX_MAX_LASER_FREQ_1_MSB_REG                            0x818E
#define AC400_TX_RX_MAX_LASER_FREQ_1_LSB_REG                            0x818F

#define AC400_TX_RX_MAX_LASER_FREQ_2_MSB_REG                            0x8190
#define AC400_TX_RX_MAX_LASER_FREQ_2_LSB_REG                            0x8191

#define AC400_TX_LASER_FTF_RANGE_MSB_REG                                0x8194
#define AC400_TX_LASER_FTF_RANGE_LSB_REG                                0x8195

#define AC400_LASER_TUNE_CAP_MSB_REG                                    0x8196
#  define AC400_LASER_TUNE_CAP_MSB_GRID_SPACING_6P25_GHZ_BIT            7
#  define AC400_LASER_TUNE_CAP_MSB_GRID_SPACING_12P5_GHZ_BIT            6
#  define AC400_LASER_TUNE_CAP_MSB_GRID_SPACING_25_GHZ_BIT              5
#  define AC400_LASER_TUNE_CAP_MSB_GRID_SPACING_33_GHZ_BIT              4
#  define AC400_LASER_TUNE_CAP_MSB_GRID_SPACING_50_GHZ_BIT              3
#  define AC400_LASER_TUNE_CAP_MSB_GRID_SPACING_100_GHZ_BIT             2
#  define AC400_LASER_TUNE_CAP_MSB_MAX_CHANS_BITS_9_TO_8_MSB            1
#  define AC400_LASER_TUNE_CAP_MSB_MAX_CHANS_BITS_9_TO_8_LSB            0

#define AC400_LASER_TUNE_CAP_LSB_REG                                    0x8197


/*------------------------------------------------------------------------------

              MSA-100GLH VR1. Command/Setup/Control/FAWS Registers
                               0xB000 - 0xB07F

------------------------------------------------------------------------------*/

#define AC400_MOD_STATE_REG                                             0xB016
#  define AC400_MOD_STATE_HI_PWR_DWN_BIT                                8
#  define AC400_MOD_STATE_TX_TURN_OFF_BIT                               7
#  define AC400_MOD_STATE_FAULT_BIT                                     6
#  define AC400_MOD_STATE_READY_BIT                                     5
#  define AC400_MOD_STATE_TX_TURN_ON_BIT                                4
#  define AC400_MOD_STATE_TX_OFF_BIT                                    3
#  define AC400_MOD_STATE_HI_PWR_UP_BIT                                 2
#  define AC400_MOD_STATE_LOW_PWR_BIT                                   1
#  define AC400_MOD_STATE_INIT_BIT                                      0

#define AC400_GLB_ALRM_SUM_REG                                          0xB018
#  define AC400_GLB_ALRM_SUM_GLB_ALRM_ASRT_STAT_BIT                     15
#  define AC400_GLB_ALRM_SUM_HOST_LANE_FLT_STAT_SUM_BIT                 14
#  define AC400_GLB_ALRM_SUM_NTWK_LANE_FLT_STAT_SUM_BIT                 13
#  define AC400_GLB_ALRM_SUM_NTWK_LANE_ALRM_WARN_1_SUM_BIT              12
#  define AC400_GLB_ALRM_SUM_MOD_ALRM_WARN_1_SUM_BIT                    10
#  define AC400_GLB_ALRM_SUM_MOD_FLT_SUM_BIT                            9
#  define AC400_GLB_ALRM_SUM_MOD_GEN_STAT_SUM_BIT                       8
#  define AC400_GLB_ALRM_SUM_MOD_STATE_SUM_BIT                          7
#  define AC400_GLB_ALRM_SUM_NTWK_LANE_ALRM_WARN_2_SUM_BIT              6
#  define AC400_GLB_ALRM_SUM_NTWK_HOST_ALRM_STAT_SUM_BIT                5
#  define AC400_GLB_ALRM_SUM_MOD_EXT_FUNCS_STAT_SUM_BIT                 4
#  define AC400_GLB_ALRM_SUM_VENDOR_SPFC_FAWS_SUM_BIT                   3
#  define AC400_GLB_ALRM_SUM_SOFT_GLB_ALRM_TEST_STAT_BIT                0

#define AC400_MOD_FLT_STAT_REG                                          0xB01E
#  define AC400_MOD_FLT_STAT_MOD_TEMP_FLT_BIT                           8
#  define AC400_MOD_FLT_STAT_SPFC_HW_FLT_BIT                            7
#  define AC400_MOD_FLT_STAT_PLD_FLASH_INIT_FLT_BIT                     6
#  define AC400_MOD_FLT_STAT_PS_FLT_BIT                                 5
#  define AC400_MOD_FLT_STAT_CHKSUM_FLT_BIT                             1

#define AC400_MOD_ALRM_WARN_1_REG                                       0xB01F
#  define AC400_MOD_ALRM_WARN_1_TEMP_HI_ALRM_BIT                        11
#  define AC400_MOD_ALRM_WARN_1_TEMP_HI_WARN_BIT                        10
#  define AC400_MOD_ALRM_WARN_1_TEMP_LO_WARN_BIT                        9
#  define AC400_MOD_ALRM_WARN_1_TEMP_LO_ALRM_BIT                        8
#  define AC400_MOD_ALRM_WARN_1_VCC_HI_ALRM_BIT                         7
#  define AC400_MOD_ALRM_WARN_1_VCC_HI_WARN_BIT                         6
#  define AC400_MOD_ALRM_WARN_1_VCC_LO_WARN_BIT                         5
#  define AC400_MOD_ALRM_WARN_1_VCC_LO_ALRM_BIT                         4

#define AC400_TEMP_MON_A2D_VAL_REG                                      0xB02F

#define AC400_MOD_PS_MON_A2D_VAL_REG                                    0xB030


/*------------------------------------------------------------------------------

            MSA-100GLH VR2. Network Lane Control/Data Registers
                               0xB300 - 0xB57F

------------------------------------------------------------------------------*/

#define AC400_NTWK_RX_CUR_IN_PWR_LN_0_REG                               0xB4E0
#define AC400_NTWK_RX_CUR_IN_PWR_LN_1_REG                               0xB4E1


/*------------------------------------------------------------------------------

           MSA-100GLH VR1. Host Lane FAWS/Control/Status Registers
                               0xB600 - 0xB6FF

------------------------------------------------------------------------------*/

#define AC400_HOST_FLT_STAT_LN_0_REG                                    0xB600
#define AC400_HOST_FLT_STAT_LN_1_REG                                    0xB601
#define AC400_HOST_FLT_STAT_LN_2_REG                                    0xB602
#define AC400_HOST_FLT_STAT_LN_3_REG                                    0xB603
#define AC400_HOST_FLT_STAT_LN_4_REG                                    0xB604
#define AC400_HOST_FLT_STAT_LN_5_REG                                    0xB605
#define AC400_HOST_FLT_STAT_LN_6_REG                                    0xB606
#define AC400_HOST_FLT_STAT_LN_7_REG                                    0xB607
#define AC400_HOST_FLT_STAT_LN_8_REG                                    0xB608
#define AC400_HOST_FLT_STAT_LN_9_REG                                    0xB609
#define AC400_HOST_FLT_STAT_LN_10_REG                                   0xB60A
#define AC400_HOST_FLT_STAT_LN_11_REG                                   0xB60B
#define AC400_HOST_FLT_STAT_LN_12_REG                                   0xB60C
#define AC400_HOST_FLT_STAT_LN_13_REG                                   0xB60D
#define AC400_HOST_FLT_STAT_LN_14_REG                                   0xB60E
#define AC400_HOST_FLT_STAT_LN_15_REG                                   0xB60F
#  define AC400_HOST_FLT_STAT_LN_TX_HOST_LOL_BIT                        0

#define AC400_HOST_PRBS_TX_ERR_CNT_LN_0_REG                             0xB630
#define AC400_HOST_PRBS_TX_ERR_CNT_LN_1_REG                             0xB631
#define AC400_HOST_PRBS_TX_ERR_CNT_LN_2_REG                             0xB632
#define AC400_HOST_PRBS_TX_ERR_CNT_LN_3_REG                             0xB633
#define AC400_HOST_PRBS_TX_ERR_CNT_LN_4_REG                             0xB634
#define AC400_HOST_PRBS_TX_ERR_CNT_LN_5_REG                             0xB635
#define AC400_HOST_PRBS_TX_ERR_CNT_LN_6_REG                             0xB636
#define AC400_HOST_PRBS_TX_ERR_CNT_LN_7_REG                             0xB637
#define AC400_HOST_PRBS_TX_ERR_CNT_LN_8_REG                             0xB638
#define AC400_HOST_PRBS_TX_ERR_CNT_LN_9_REG                             0xB639
#define AC400_HOST_PRBS_TX_ERR_CNT_LN_10_REG                            0xB63A
#define AC400_HOST_PRBS_TX_ERR_CNT_LN_11_REG                            0xB63B
#define AC400_HOST_PRBS_TX_ERR_CNT_LN_12_REG                            0xB63C
#define AC400_HOST_PRBS_TX_ERR_CNT_LN_13_REG                            0xB63D
#define AC400_HOST_PRBS_TX_ERR_CNT_LN_14_REG                            0xB63E
#define AC400_HOST_PRBS_TX_ERR_CNT_LN_15_REG                            0xB63F
#  define AC400_HOST_PRBS_TX_ERR_CNT_LN_EXPONENT_MSB                    15
#  define AC400_HOST_PRBS_TX_ERR_CNT_LN_EXPONENT_LSB                    10
#  define AC400_HOST_PRBS_TX_ERR_CNT_LN_MANTISSA_MSB                    9
#  define AC400_HOST_PRBS_TX_ERR_CNT_LN_MANTISSA_LSB                    0

#define AC400_CLIENT_TX_ALGN_STAT_INTF_0_REG                            0xB650
#define AC400_CLIENT_TX_ALGN_STAT_INTF_1_REG                            0xB651
#define AC400_CLIENT_TX_ALGN_STAT_INTF_2_REG                            0xB652
#define AC400_CLIENT_TX_ALGN_STAT_INTF_3_REG                            0xB653
#  define AC400_CLIENT_TX_ALGN_STAT_INTF_CDR_LOCK_FLT_BIT               15
#  define AC400_CLIENT_TX_ALGN_STAT_INTF_LOSS_OF_ALGN_BIT               14
#  define AC400_CLIENT_TX_ALGN_STAT_INTF_OUT_OF_ALGN_BIT                13
#  define AC400_CLIENT_TX_ALGN_STAT_INTF_DESKW_LCK_FLT_BIT              12


/*------------------------------------------------------------------------------

             AC400 Specific Management Interface (Vendor Private Use)
                               0x9000 - 0x9FFF

------------------------------------------------------------------------------*/

#define AC400_NTWK_TX_TURN_UP_STATE_LN_0_REG                            0x9110
#define AC400_NTWK_TX_TURN_UP_STATE_LN_1_REG                            0x9111
#  define AC400_NTWK_TX_TURN_UP_STATE_LN_TX_OUT_PWR_ADJ_BIT             5
#  define AC400_NTWK_TX_TURN_UP_STATE_LN_TX_MODULATOR_CONVERGE_BIT      4
#  define AC400_NTWK_TX_TURN_UP_STATE_LN_TX_LASER_READY_BIT             3
#  define AC400_NTWK_TX_TURN_UP_STATE_LN_TX_LAS_READY_OFF_BIT           2
#  define AC400_NTWK_TX_TURN_UP_STATE_LN_ASIC_TX_READY_BIT              1
#  define AC400_NTWK_TX_TURN_UP_STATE_LN_TX_INIT_BIT                    0

#define AC400_NTWK_RX_TURN_UP_STATE_LN_0_REG                            0x9118
#define AC400_NTWK_RX_TURN_UP_STATE_LN_1_REG                            0x9119
#  define AC400_NTWK_RX_TURN_UP_STATE_LN_RX_DEMOD_LOCK_BIT              5
#  define AC400_NTWK_RX_TURN_UP_STATE_LN_DISPERSION_LOCK_BIT            4
#  define AC400_NTWK_RX_TURN_UP_STATE_LN_ADC_OUTPUT_BIT                 3
#  define AC400_NTWK_RX_TURN_UP_STATE_LN_OPTICAL_INPUT_BIT              2
#  define AC400_NTWK_RX_TURN_UP_STATE_LN_ASIC_RX_READY_BIT              1
#  define AC400_NTWK_RX_TURN_UP_STATE_LN_RX_INIT_BIT                    0

#define AC400_NTWK_ALRM_WARN_1_LN_0_REG                                 0xB180
#define AC400_NTWK_ALRM_WARN_1_LN_1_REG                                 0xB181
#  define AC400_NTWK_ALRM_WARN_1_LN_BIAS_HI_ALRM_BIT                    15
#  define AC400_NTWK_ALRM_WARN_1_LN_BIAS_HI_WARN_BIT                    14
#  define AC400_NTWK_ALRM_WARN_1_LN_BIAS_LO_WARN_BIT                    13
#  define AC400_NTWK_ALRM_WARN_1_LN_BIAS_LO_ALRM_BIT                    12
#  define AC400_NTWK_ALRM_WARN_1_LN_TX_PWR_HI_ALRM_BIT                  11
#  define AC400_NTWK_ALRM_WARN_1_LN_TX_PWR_HI_WARN_BIT                  10
#  define AC400_NTWK_ALRM_WARN_1_LN_TX_PWR_LO_WARN_BIT                  9
#  define AC400_NTWK_ALRM_WARN_1_LN_TX_PWR_LO_ALRM_BIT                  8
#  define AC400_NTWK_ALRM_WARN_1_LN_LAS_TEMP_HI_ALRM_BIT                7
#  define AC400_NTWK_ALRM_WARN_1_LN_LAS_TEMP_HI_WARN_BIT                6
#  define AC400_NTWK_ALRM_WARN_1_LN_LAS_TEMP_LO_WARN_BIT                5
#  define AC400_NTWK_ALRM_WARN_1_LN_LAS_TEMP_LO_ALRM_BIT                4
#  define AC400_NTWK_ALRM_WARN_1_LN_RX_PWR_HI_ALRM_BIT                  3
#  define AC400_NTWK_ALRM_WARN_1_LN_RX_PWR_HI_WARN_BIT                  2
#  define AC400_NTWK_ALRM_WARN_1_LN_RX_PWR_LO_WARN_BIT                  1
#  define AC400_NTWK_ALRM_WARN_1_LN_RX_PWR_LO_ALRM_BIT                  0

#define AC400_NTWK_ALRM_WARN_2_LN_0_REG                                 0xB190
#define AC400_NTWK_ALRM_WARN_2_LN_1_REG                                 0xB191
#  define AC400_NTWK_ALRM_WARN_2_LN_TX_MOD_BIAS_HI_ALRM_BIT             3
#  define AC400_NTWK_ALRM_WARN_2_LN_TX_MOD_BIAS_HI_WARN_BIT             2
#  define AC400_NTWK_ALRM_WARN_2_LN_TX_MOD_BIAS_LO_WARN_BIT             1
#  define AC400_NTWK_ALRM_WARN_2_LN_TX_MOD_BIAS_LO_ALRM_BIT             0

#define AC400_NTWK_TX_ALGN_STAT_LN_0_REG                                0xB210
#define AC400_NTWK_TX_ALGN_STAT_LN_1_REG                                0xB211
#  define AC400_NTWK_TX_ALGN_STAT_LN_OUT_OF_ALGN_BIT                    14
#  define AC400_NTWK_TX_ALGN_STAT_LN_CMU_LCK_FLT_BIT                    13
#  define AC400_NTWK_TX_ALGN_STAT_LN_REF_CLK_FLT_BIT                    12

#define AC400_NTWK_RX_ALGN_STAT_LN_0_REG                                0xB250
#define AC400_NTWK_RX_ALGN_STAT_LN_1_REG                                0xB251
#  define AC400_NTWK_RX_ALGN_STAT_LN_MODEM_SYNC_DET_FLT_BIT             15
#  define AC400_NTWK_RX_ALGN_STAT_LN_MODEM_LOCK_FLT_BIT                 14
#  define AC400_NTWK_RX_ALGN_STAT_LN_LOSS_OF_ALGN_FLT_BIT               13
#  define AC400_NTWK_RX_ALGN_STAT_LN_OUT_OF_ALGN_FLT_BIT                12
#  define AC400_NTWK_RX_ALGN_STAT_LN_TIMING_FLT_BIT                     11

#define AC400_NTWK_PRBS_RX_ERR_CNT_LN_0_REG                             0xB310
#define AC400_NTWK_PRBS_RX_ERR_CNT_LN_1_REG                             0xB311
#  define AC400_NTWK_PRBS_RX_ERR_CNT_LN_EXPONENT_MSB                    15
#  define AC400_NTWK_PRBS_RX_ERR_CNT_LN_EXPONENT_LSB                    10
#  define AC400_NTWK_PRBS_RX_ERR_CNT_LN_MANTISSA_MSB                    9
#  define AC400_NTWK_PRBS_RX_ERR_CNT_LN_MANTISSA_LSB                    0

#define AC400_CLIENT_SERDES_PRBS_DATA_CNT_INTF_0_REG                    0x9238
#define AC400_CLIENT_SERDES_PRBS_DATA_CNT_INTF_1_REG                    0x9239
#define AC400_CLIENT_SERDES_PRBS_DATA_CNT_INTF_2_REG                    0x923A
#define AC400_CLIENT_SERDES_PRBS_DATA_CNT_INTF_3_REG                    0x923B
#  define AC400_CLIENT_SERDES_PRBS_DATA_CNT_INTF_EXPONENT_MSB           15
#  define AC400_CLIENT_SERDES_PRBS_DATA_CNT_INTF_EXPONENT_LSB           10
#  define AC400_CLIENT_SERDES_PRBS_DATA_CNT_INTF_MANTISSA_MSB           9
#  define AC400_CLIENT_SERDES_PRBS_DATA_CNT_INTF_MANTISSA_LSB           0

#define AC400_CLIENT_FRAMED_PRBS_DATA_CNT_INTF_0_REG                    0x9248
#define AC400_CLIENT_FRAMED_PRBS_DATA_CNT_INTF_1_REG                    0x9249
#define AC400_CLIENT_FRAMED_PRBS_DATA_CNT_INTF_2_REG                    0x924A
#define AC400_CLIENT_FRAMED_PRBS_DATA_CNT_INTF_3_REG                    0x924B
#  define AC400_CLIENT_FRAMED_PRBS_DATA_CNT_INTF_EXPONENT_MSB           15
#  define AC400_CLIENT_FRAMED_PRBS_DATA_CNT_INTF_EXPONENT_LSB           10
#  define AC400_CLIENT_FRAMED_PRBS_DATA_CNT_INTF_MANTISSA_MSB           9
#  define AC400_CLIENT_FRAMED_PRBS_DATA_CNT_INTF_MANTISSA_LSB           0

#define AC400_CLIENT_FRAMED_PRBS_TX_ERR_CNT_INTF_0_REG                  0x9250
#define AC400_CLIENT_FRAMED_PRBS_TX_ERR_CNT_INTF_1_REG                  0x9251
#define AC400_CLIENT_FRAMED_PRBS_TX_ERR_CNT_INTF_2_REG                  0x9252
#define AC400_CLIENT_FRAMED_PRBS_TX_ERR_CNT_INTF_3_REG                  0x9253
#  define AC400_CLIENT_FRAMED_PRBS_TX_ERR_CNT_INTF_EXPONENT_MSB         15
#  define AC400_CLIENT_FRAMED_PRBS_TX_ERR_CNT_INTF_EXPONENT_LSB         10
#  define AC400_CLIENT_FRAMED_PRBS_TX_ERR_CNT_INTF_MANTISSA_MSB         9
#  define AC400_CLIENT_FRAMED_PRBS_TX_ERR_CNT_INTF_MANTISSA_LSB         0

#define AC400_CLIENT_FRAMED_PRBS_SYNC_STAT_INTF_0_REG                   0x9258
#define AC400_CLIENT_FRAMED_PRBS_SYNC_STAT_INTF_1_REG                   0x9259
#define AC400_CLIENT_FRAMED_PRBS_SYNC_STAT_INTF_2_REG                   0x925A
#define AC400_CLIENT_FRAMED_PRBS_SYNC_STAT_INTF_3_REG                   0x925B
#  define AC400_CLIENT_FRAMED_PRBS_SYNC_STAT_INTF_FRAME_SYNC_BIT        1
#  define AC400_CLIENT_FRAMED_PRBS_SYNC_STAT_INTF_CHECK_SYNC_BIT        0

#define AC400_CLIENT_FRAMED_PRBS_LSYNC_WH_INTF_0_REG                    0x9260
#define AC400_CLIENT_FRAMED_PRBS_LSYNC_WL_INTF_0_REG                    0x9261

#define AC400_CLIENT_FRAMED_PRBS_LSYNC_WH_INTF_1_REG                    0x9262
#define AC400_CLIENT_FRAMED_PRBS_LSYNC_WL_INTF_1_REG                    0x9263

#define AC400_CLIENT_FRAMED_PRBS_LSYNC_WH_INTF_2_REG                    0x9264
#define AC400_CLIENT_FRAMED_PRBS_LSYNC_WL_INTF_2_REG                    0x9265

#define AC400_CLIENT_FRAMED_PRBS_LSYNC_WH_INTF_3_REG                    0x9266
#define AC400_CLIENT_FRAMED_PRBS_LSYNC_WL_INTF_3_REG                    0x9267

#define AC400_CLIENT_PCS_ALRM_STAT_INTF_0_REG                           0x92A0
#define AC400_CLIENT_PCS_ALRM_STAT_INTF_1_REG                           0x92A1
#define AC400_CLIENT_PCS_ALRM_STAT_INTF_2_REG                           0x92A2
#define AC400_CLIENT_PCS_ALRM_STAT_INTF_3_REG                           0x92A3
#  define AC400_CLIENT_PCS_ALRM_STAT_INTF_OPU_CSF_BIT                   12
#  define AC400_CLIENT_PCS_ALRM_STAT_INTF_INGRESS_LOCAL_FLT_BIT         11
#  define AC400_CLIENT_PCS_ALRM_STAT_INTF_INGRESS_LOA_BIT               10
#  define AC400_CLIENT_PCS_ALRM_STAT_INTF_INGRESS_AM_LCK_FLT_BIT        9
#  define AC400_CLIENT_PCS_ALRM_STAT_INTF_INGRESS_BLK_LCK_FLT_BIT       8
#  define AC400_CLIENT_PCS_ALRM_STAT_INTF_EGRESS_HI_BER_BIT             4
#  define AC400_CLIENT_PCS_ALRM_STAT_INTF_EGRESS_LOCAL_FLT_BIT          3
#  define AC400_CLIENT_PCS_ALRM_STAT_INTF_EGRESS_LOA_BIT                2
#  define AC400_CLIENT_PCS_ALRM_STAT_INTF_EGRESS_AM_LCK_FLT_BIT         1
#  define AC400_CLIENT_PCS_ALRM_STAT_INTF_EGRESS_BLK_LCK_FLT_BIT        0

#define AC400_NTWK_CUR_BER_WH_LN_0_REG                                  0x9300
#define AC400_NTWK_CUR_BER_WL_LN_0_REG                                  0x9301

#define AC400_NTWK_CUR_BER_WH_LN_1_REG                                  0x9302
#define AC400_NTWK_CUR_BER_WL_LN_1_REG                                  0x9303

#define AC400_NTWK_FEC_ACCUM_CNTS_CNTL_LN_0_REG                         0x9400
#define AC400_NTWK_FEC_ACCUM_CNTS_CNTL_LN_1_REG                         0x9401
#  define AC400_NTWK_FEC_ACCUM_CNTS_CNTL_LN_RST_ALL_ACCUM_COUNTS_BIT    0

#define AC400_NTWK_FEC_UNCORR_CB_CNT_W1_LN_0_REG                        0x9480
#define AC400_NTWK_FEC_UNCORR_CB_CNT_W2_LN_0_REG                        0x9481
#define AC400_NTWK_FEC_UNCORR_CB_CNT_W3_LN_0_REG                        0x9482
#define AC400_NTWK_FEC_UNCORR_CB_CNT_W4_LN_0_REG                        0x9483

#define AC400_NTWK_FEC_UNCORR_CB_CNT_W1_LN_1_REG                        0x9484
#define AC400_NTWK_FEC_UNCORR_CB_CNT_W2_LN_1_REG                        0x9485
#define AC400_NTWK_FEC_UNCORR_CB_CNT_W3_LN_1_REG                        0x9486
#define AC400_NTWK_FEC_UNCORR_CB_CNT_W4_LN_1_REG                        0x9487

#define AC400_IND_NTWK_LANE_TX_DIS_CNTL_REG                             0xB013
#  define AC400_IND_NTWK_LANE_TX_DIS_CNTL_TX_DIS_LANE_1_BIT             1
#  define AC400_IND_NTWK_LANE_TX_DIS_CNTL_TX_DIS_LANE_0_BIT             0

#define AC400_NTWK_TX_CHAN_CNTL_LN_0_REG                                0xB400
#define AC400_NTWK_TX_CHAN_CNTL_LN_1_REG                                0xB401
#  define AC400_NTWK_TX_CHAN_CNTL_LN_GRID_SPACING_MSB                   15
#  define AC400_NTWK_TX_CHAN_CNTL_LN_GRID_SPACING_LSB                   13
#  define AC400_NTWK_TX_CHAN_CNTL_LN_CHAN_NUM_MSB                       9
#  define AC400_NTWK_TX_CHAN_CNTL_LN_CHAN_NUM_LSB                       0

#define AC400_NTWK_TX_OUTPUT_PWR_LN_0_REG                               0xB410
#define AC400_NTWK_TX_OUTPUT_PWR_LN_1_REG                               0xB411

#define AC400_NTWK_TX_LASER_FTF_LN_0_REG                                0xB430
#define AC400_NTWK_TX_LASER_FTF_LN_1_REG                                0xB431

#define AC400_NTWK_TX_FREQ_1_LN_0_REG                                   0xB450
#define AC400_NTWK_TX_FREQ_1_LN_1_REG                                   0xB451

#define AC400_NTWK_TX_FREQ_2_LN_0_REG                                   0xB460
#define AC400_NTWK_TX_FREQ_2_LN_1_REG                                   0xB461

#define AC400_NTWK_TX_CUR_OUT_PWR_LN_0_REG                              0xB4A0
#define AC400_NTWK_TX_CUR_OUT_PWR_LN_1_REG                              0xB4A1

#define AC400_DEVICE_SETUP_CNTL_REG                                     0x9010
#  define AC400_DEVICE_SETUP_CNTL_DEV_CFG_MSB                           1
#  define AC400_DEVICE_SETUP_CNTL_DEV_CFG_LSB                           0

#define AC400_NTWK_LANES_ENABLE_CNTL_REG                                0x9018
#  define AC400_NTWK_LANES_ENABLE_CNTL_MASTER_EN_LN_1_BIT               1
#  define AC400_NTWK_LANES_ENABLE_CNTL_MASTER_EN_LN_0_BIT               0

#define AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_REG                       0x9020
#  define AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_INTF_3_EN_BIT           15
#  define AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_INTF_2_EN_BIT           14
#  define AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_INTF_1_EN_BIT           13
#  define AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_INTF_0_EN_BIT           12
#  define AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_INTF_3_LANES_12_TO_15_RATE_SEL_MSB 11
#  define AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_INTF_3_LANES_12_TO_15_RATE_SEL_LSB 9
#  define AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_INTF_2_LANES_8_TO_11_RATE_SEL_MSB 8
#  define AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_INTF_2_LANES_8_TO_11_RATE_SEL_LSB 6
#  define AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_INTF_1_LANES_4_TO_7_RATE_SEL_MSB 5
#  define AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_INTF_1_LANES_4_TO_7_RATE_SEL_LSB 3
#  define AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_INTF_0_LANES_0_TO_3_RATE_SEL_MSB 2
#  define AC400_HOST_LANES_CLIENT_INTF_DEF_CNTL_INTF_0_LANES_0_TO_3_RATE_SEL_LSB 0

#define AC400_NTWK_GEN_MODE_CNTL_LN_0_REG                               0x9100
#define AC400_NTWK_GEN_MODE_CNTL_LN_1_REG                               0x9101
#  define AC400_NTWK_GEN_MODE_CNTL_LN_MODULATION_FORMAT_MSB             11
#  define AC400_NTWK_GEN_MODE_CNTL_LN_MODULATION_FORMAT_LSB             8
#  define AC400_NTWK_GEN_MODE_CNTL_LN_NON_DIFF_EN_BIT                   7
#  define AC400_NTWK_GEN_MODE_CNTL_LN_FEC_MODE_MSB                      2
#  define AC400_NTWK_GEN_MODE_CNTL_LN_FEC_MODE_LSB                      0

#define AC400_NTWK_TX_CNTL_LN_0_REG                                     0x9120
#define AC400_NTWK_TX_CNTL_LN_1_REG                                     0x9121
#  define AC400_NTWK_TX_CNTL_LN_PRBS_GEN_EN_BIT                         14
#  define AC400_NTWK_TX_CNTL_LN_PRBS_GEN_PAT_MSB                        13
#  define AC400_NTWK_TX_CNTL_LN_PRBS_GEN_PAT_LSB                        12
#  define AC400_NTWK_TX_CNTL_LN_PRBS_GEN_INV_PAT_BIT                    11
#  define AC400_NTWK_TX_CNTL_LN_TX_FIFO_RST_BIT                         10
#  define AC400_NTWK_TX_CNTL_LN_TX_RESET_BIT                            8
#  define AC400_NTWK_TX_CNTL_LN_TX_MCLK_CNTL_MSB                        7
#  define AC400_NTWK_TX_CNTL_LN_TX_MCLK_CNTL_LSB                        5
#  define AC400_NTWK_TX_CNTL_LN_TX_REF_CLK_RATE_SEL_MSB                 4
#  define AC400_NTWK_TX_CNTL_LN_TX_REF_CLK_RATE_SEL_LSB                 2
#  define AC400_NTWK_TX_CNTL_LN_TX_AIS_MODE_MSB                         1
#  define AC400_NTWK_TX_CNTL_LN_TX_AIS_MODE_LSB                         0

#define AC400_NTWK_RX_CNTL_LN_0_REG                                     0x9128
#define AC400_NTWK_RX_CNTL_LN_1_REG                                     0x9129
#  define AC400_NTWK_RX_CNTL_LN_PRBS_CHK_EN_BIT                         14
#  define AC400_NTWK_RX_CNTL_LN_PRBS_CHK_PAT_MSB                        13
#  define AC400_NTWK_RX_CNTL_LN_PRBS_CHK_PAT_LSB                        12
#  define AC400_NTWK_RX_CNTL_LN_PRBS_CHK_INV_PAT_BIT                    11
#  define AC400_NTWK_RX_CNTL_LN_LOOPBACK_EN_BIT                         10
#  define AC400_NTWK_RX_CNTL_LN_RX_RESET_BIT                            8
#  define AC400_NTWK_RX_CNTL_LN_RX_FIFO_RST_BIT                         4

#define AC400_CLIENT_GEN_CNTL_INTF_0_REG                                0x9200
#define AC400_CLIENT_GEN_CNTL_INTF_1_REG                                0x9201
#define AC400_CLIENT_GEN_CNTL_INTF_2_REG                                0x9202
#define AC400_CLIENT_GEN_CNTL_INTF_3_REG                                0x9203
#  define AC400_CLIENT_GEN_CNTL_INTF_TX_FEC_DECODER_DIS_BIT             15
#  define AC400_CLIENT_GEN_CNTL_INTF_TX_RESET_BIT                       11
#  define AC400_CLIENT_GEN_CNTL_INTF_LOOPBACK_EN_BIT                    10
#  define AC400_CLIENT_GEN_CNTL_INTF_RX_FEC_ENCODER_DIS_BIT             7
#  define AC400_CLIENT_GEN_CNTL_INTF_RX_RESET_BIT                       5
#  define AC400_CLIENT_GEN_CNTL_INTF_RX_MCLK_CNTL_MSB                   4
#  define AC400_CLIENT_GEN_CNTL_INTF_RX_MCLK_CNTL_LSB                   2
#  define AC400_CLIENT_GEN_CNTL_INTF_RX_AIS_MODE_MSB                    1
#  define AC400_CLIENT_GEN_CNTL_INTF_RX_AIS_MODE_LSB                    0

#define AC400_CLIENT_SERDES_LANES_PRBS_CNTL_INTF_0_REG                  0x9210
#define AC400_CLIENT_SERDES_LANES_PRBS_CNTL_INTF_1_REG                  0x9211
#define AC400_CLIENT_SERDES_LANES_PRBS_CNTL_INTF_2_REG                  0x9212
#define AC400_CLIENT_SERDES_LANES_PRBS_CNTL_INTF_3_REG                  0x9213
#  define AC400_CLIENT_SERDES_LANES_PRBS_CNTL_INTF_TX_CHK_EN_BIT        14
#  define AC400_CLIENT_SERDES_LANES_PRBS_CNTL_INTF_TX_CHK_PAT_MSB       13
#  define AC400_CLIENT_SERDES_LANES_PRBS_CNTL_INTF_TX_CHK_PAT_LSB       12
#  define AC400_CLIENT_SERDES_LANES_PRBS_CNTL_INTF_RX_GEN_EN_BIT        7
#  define AC400_CLIENT_SERDES_LANES_PRBS_CNTL_INTF_RX_GEN_PAT_MSB       6
#  define AC400_CLIENT_SERDES_LANES_PRBS_CNTL_INTF_RX_GEN_PAT_LSB       5

#define AC400_CLIENT_HOST_TX_EQUAL_CNTL_INTF_0_REG                      0x9218
#define AC400_CLIENT_HOST_TX_EQUAL_CNTL_INTF_1_REG                      0x9219
#define AC400_CLIENT_HOST_TX_EQUAL_CNTL_INTF_2_REG                      0x921A
#define AC400_CLIENT_HOST_TX_EQUAL_CNTL_INTF_3_REG                      0x921B
#  define AC400_CLIENT_HOST_TX_EQUAL_CNTL_INTF_LF_CTLE_MSB              15
#  define AC400_CLIENT_HOST_TX_EQUAL_CNTL_INTF_LF_CTLE_LSB              12
#  define AC400_CLIENT_HOST_TX_EQUAL_CNTL_INTF_CTLE_MSB                 11
#  define AC400_CLIENT_HOST_TX_EQUAL_CNTL_INTF_CTLE_LSB                 7
#  define AC400_CLIENT_HOST_TX_EQUAL_CNTL_INTF_DFE_MSB                  5
#  define AC400_CLIENT_HOST_TX_EQUAL_CNTL_INTF_DFE_LSB                  0

#define AC400_CLIENT_HOST_RX_TAP_0_CNTL_INTF_0_REG                      0x9220
#define AC400_CLIENT_HOST_RX_TAP_0_CNTL_INTF_1_REG                      0x9221
#define AC400_CLIENT_HOST_RX_TAP_0_CNTL_INTF_2_REG                      0x9222
#define AC400_CLIENT_HOST_RX_TAP_0_CNTL_INTF_3_REG                      0x9223
#  define AC400_CLIENT_HOST_RX_TAP_0_CNTL_INTF_GAIN_MSB                 6
#  define AC400_CLIENT_HOST_RX_TAP_0_CNTL_INTF_GAIN_LSB                 4
#  define AC400_CLIENT_HOST_RX_TAP_0_CNTL_INTF_DELAY_MSB                2
#  define AC400_CLIENT_HOST_RX_TAP_0_CNTL_INTF_DELAY_LSB                0

#define AC400_CLIENT_HOST_RX_TAP_1_CNTL_INTF_0_REG                      0x9228
#define AC400_CLIENT_HOST_RX_TAP_1_CNTL_INTF_1_REG                      0x9229
#define AC400_CLIENT_HOST_RX_TAP_1_CNTL_INTF_2_REG                      0x922A
#define AC400_CLIENT_HOST_RX_TAP_1_CNTL_INTF_3_REG                      0x922B
#  define AC400_CLIENT_HOST_RX_TAP_1_CNTL_INTF_GAIN_MSB                 6
#  define AC400_CLIENT_HOST_RX_TAP_1_CNTL_INTF_GAIN_LSB                 4

#define AC400_CLIENT_HOST_RX_TAP_2_CNTL_INTF_0_REG                      0x9230
#define AC400_CLIENT_HOST_RX_TAP_2_CNTL_INTF_1_REG                      0x9231
#define AC400_CLIENT_HOST_RX_TAP_2_CNTL_INTF_2_REG                      0x9232
#define AC400_CLIENT_HOST_RX_TAP_2_CNTL_INTF_3_REG                      0x9233
#  define AC400_CLIENT_HOST_RX_TAP_2_CNTL_INTF_GAIN_MSB                 7
#  define AC400_CLIENT_HOST_RX_TAP_2_CNTL_INTF_GAIN_LSB                 4
#  define AC400_CLIENT_HOST_RX_TAP_2_CNTL_INTF_DELAY_MSB                2
#  define AC400_CLIENT_HOST_RX_TAP_2_CNTL_INTF_DELAY_LSB                0

#define AC400_CLIENT_FRAMED_PRBS_CNTL_INTF_0_REG                        0x9240
#define AC400_CLIENT_FRAMED_PRBS_CNTL_INTF_1_REG                        0x9241
#define AC400_CLIENT_FRAMED_PRBS_CNTL_INTF_2_REG                        0x9242
#define AC400_CLIENT_FRAMED_PRBS_CNTL_INTF_3_REG                        0x9243
#  define AC400_CLIENT_FRAMED_PRBS_CNTL_INTF_TX_CHK_EN_BIT              14
#  define AC400_CLIENT_FRAMED_PRBS_CNTL_INTF_TX_CHK_PAT_MSB             13
#  define AC400_CLIENT_FRAMED_PRBS_CNTL_INTF_TX_CHK_PAT_LSB             12
#  define AC400_CLIENT_FRAMED_PRBS_CNTL_INTF_TX_CHK_INV_PAT_BIT         11
#  define AC400_CLIENT_FRAMED_PRBS_CNTL_INTF_TX_CHK_SCRAM_DIS_BIT       10
#  define AC400_CLIENT_FRAMED_PRBS_CNTL_INTF_RX_GEN_EN_BIT              7
#  define AC400_CLIENT_FRAMED_PRBS_CNTL_INTF_RX_GEN_PAT_MSB             6
#  define AC400_CLIENT_FRAMED_PRBS_CNTL_INTF_RX_GEN_PAT_LSB             5
#  define AC400_CLIENT_FRAMED_PRBS_CNTL_INTF_RX_GEN_INV_PAT_BIT         4
#  define AC400_CLIENT_FRAMED_PRBS_CNTL_INTF_RX_GEN_SCRAM_DIS_BIT       3
#  define AC400_CLIENT_FRAMED_PRBS_CNTL_INTF_Mill_BIT                   3

#endif /* AC400_H__ */
