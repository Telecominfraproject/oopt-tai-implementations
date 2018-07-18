/**
 * @file    voyager_tai_adapter.h
 * @brief   Voyager TAI adapter defines
 * @author  Scott Emery <scotte@cumulusnetworks.com>
 *
 * @copyright Copyright (C) 2018 Cumulus Networks, Inc. All rights reserved
 *
 * @remark  This source code is licensed under the BSD 3-Clause license found in
 *          the LICENSE file in the root directory of this source tree.
 */

#ifndef VOYAGER_TAI_ADAPTER_H__
#define VOYAGER_TAI_ADAPTER_H__

#include "tai.h"
#include "ac400.h"
#include "voyager.h"

/*
 *  Support for custom TAI module attributes for the AC400
 */
/**
 * @brief Configuration modes for the network interfaces
 */
typedef enum _tai_module_network_mode_t
{
    TAI_MODULE_NETWORK_MODE_UNKNOWN,
    TAI_MODULE_NETWORK_MODE_INDEPENDENT,
    TAI_MODULE_NETWORK_MODE_COUPLED,
    TAI_MODULE_NETWORK_MODE_MAX
} tai_module_network_mode_t;

/**
 * @brief Attribute Id in tai_set_module_attribute() and
 *        tai_get_module_attribute() calls for AC400 custom attributes.
 */
typedef enum _tai_module_ac400_custom_attr_t
{
    /**
     * @brief The operational mode of the network interfaces
     *
     * @type #tai_module_network_mode_t
     */
    TAI_MODULE_ATTR_NETWORK_MODE = TAI_MODULE_ATTR_CUSTOM_RANGE_START
} tai_module_ac400_custom_attr_t;

/*
 *  Support for custom TAI network interface attributes for the AC400
 */
/** @brief The transmit turn-up state */
typedef enum _tai_network_interface_tx_turn_up_state_t
{
    TAI_NETWORK_INTERFACE_TX_TURN_UP_PATH_INIT      = 0x01,
    TAI_NETWORK_INTERFACE_TX_TURN_UP_DATA_PATH      = 0x02,
    TAI_NETWORK_INTERFACE_TX_TURN_UP_LASER_OFF      = 0x04,
    TAI_NETWORK_INTERFACE_TX_TURN_UP_LASER_READY    = 0x08,
    TAI_NETWORK_INTERFACE_TX_TURN_UP_MODULATOR_CONV = 0x10,
    TAI_NETWORK_INTERFACE_TX_TURN_UP_POWER_ADJUST   = 0x20
} tai_network_interface_tx_turn_up_state_t;

/** @brief The receive turn-up state */
typedef enum _tai_network_interface_rx_turn_up_state_t
{
    TAI_NETWORK_INTERFACE_RX_TURN_UP_PATH_INIT      = 0x01,
    TAI_NETWORK_INTERFACE_RX_TURN_UP_DATA_PATH      = 0x02,
    TAI_NETWORK_INTERFACE_RX_TURN_UP_OPTICAL_SIGNAL = 0x04,
    TAI_NETWORK_INTERFACE_RX_TURN_UP_ADC_OUTPUT     = 0x08,
    TAI_NETWORK_INTERFACE_RX_TURN_UP_GOOD_DISP      = 0x10,
    TAI_NETWORK_INTERFACE_RX_TURN_UP_DEMOD_LOCK     = 0x20
} tai_network_interface_rx_turn_up_state_t;

/** @brief The forward error correction mode */
typedef enum _tai_network_interface_fec_mode_t
{
    TAI_NETWORK_INTERFACE_FEC_MODE_UNKNOWN,
    TAI_NETWORK_INTERFACE_FEC_MODE_15,
    TAI_NETWORK_INTERFACE_FEC_MODE_15_NON_STD,
    TAI_NETWORK_INTERFACE_FEC_MODE_25,
    TAI_NETWORK_INTERFACE_FEC_MODE_MAX
} tai_network_interface_fec_mode_t;

/**
 * @brief Attribute Id in tai_set_network_interface_attribute() and 
 *        tai_get_network_interface_attribute() calls for AC400 custom
 *        attributes.
 */
typedef enum _tai_network_interface_ac400_custom_attr_t
{
    /**
     * @brief The transmit turn-up state
     *
     * A bit in this attribute is set for each state successfully completed 
     * during TX turn-up. 
     *
     * @type #tai_network_interface_tx_turn_up_state_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_TX_TURN_UP_STATE = TAI_NETWORK_INTERFACE_ATTR_CUSTOM_RANGE_START,

    /**
     * @brief The receive turn-up state
     *
     * A bit in this attribute is set for each state successfully completed 
     * during RX turn-up. 
     *
     * @type #tai_network_interface_rx_turn_up_state_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_RX_TURN_UP_STATE,

    /**
     * @brief Clear FEC accumuluative counters
     *
     * @type bool
     */
    TAI_NETWORK_INTERFACE_ATTR_CLEAR_FEC_COUNTERS,

    /**
     * @brief FEC Uncorrectable code blocks since reset
     *
     * @type #tai_uint64_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_FEC_UNCORRECTABLE,

    /**
     * @brief Master Enable, when enabled the modem will turn-up if low power is 
     *        de-asserted.
     *
     * @type bool
     */
    TAI_NETWORK_INTERFACE_ATTR_MASTER_ENABLE,

    /**
     * @brief FEC mode
     *
     * @type #tai_network_interface_fec_mode_t
     */
    TAI_NETWORK_INTERFACE_ATTR_FEC_MODE,

    /**
     * @brief TX Reset
     *
     * @type bool
     */
    TAI_NETWORK_INTERFACE_ATTR_TX_RESET,

    /**
     * @brief TX FIFO Reset
     *
     * @type bool
     */
    TAI_NETWORK_INTERFACE_ATTR_TX_FIFO_RESET,

    /**
     * @brief RX Reset
     *
     * @type bool
     */
    TAI_NETWORK_INTERFACE_ATTR_RX_RESET,

    /**
     * @brief RX FIFO Reset
     *
     * @type bool
     */
    TAI_NETWORK_INTERFACE_ATTR_RX_FIFO_RESET,

    /**
     * @brief The current measured RX input power in dBm
     *
     * @type #tai_float_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_CURRENT_INPUT_POWER,

    /**
     * @brief The TX laser fine tune frequency range in Hz
     *
     * @type #tai_uint64_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_FINE_TUNE_LASER_FREQ,


    /**
     * @brief The maximum number of laser tuning channels
     *
     * @type #tai_uint32_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_MAX_LASER_CHANNELS

} tai_network_interface_ac400_custom_attr_t;

/*
 *  Support for custom TAI host interface attributes for the AC400
 */
/** @brief The interface rate */
typedef enum _tai_host_interface_rate_t
{
    TAI_HOST_INTERFACE_RATE_OTU4_27_95G,
    TAI_HOST_INTERFACE_RATE_100GE_25_78G
} tai_host_interface_rate_t;

/**
 * @brief Attribute Id in tai_set_host_interface_attribute() and 
 *        tai_get_host_interface_attribute() calls for AC400 custom
 *        attributes.
 */
typedef enum _tai_host_interface_ac400_custom_attr_t
{
    /**
     * @brief Speed of the host interface
     *
     * @type #tai_host_interface_rate_t
     */
    TAI_HOST_INTERFACE_ATTR_RATE = TAI_HOST_INTERFACE_ATTR_CUSTOM_RANGE_START,

    /**
     * @brief Host interface enable
     *
     *  Enables (true) or disables (false) a host interface.
     *
     * @type bool
     */
    TAI_HOST_INTERFACE_ATTR_ENABLE,

    /**
     * @brief FEC decoder enable
     *
     *  Enables (true) or disables (false) the FEC decoder (host to module)
     *
     * @type bool
     */
    TAI_HOST_INTERFACE_ATTR_FEC_DECODER,

    /**
     * @brief FEC encoder enable
     *
     *  Enables (true) or disables (false) the FEC encoder (module to host)
     *
     * @type bool
     */
    TAI_HOST_INTERFACE_ATTR_FEC_ENCODER,

    /**
     * @brief TX reset
     *
     *  Enables (true) or disables (false) the TX host interface reset
     *
     * @type bool
     */
    TAI_HOST_INTERFACE_ATTR_TX_RESET,

    /**
     * @brief RX reset
     *
     *  Enables (true) or disables (false) the RX host interface reset
     *
     * @type bool
     */
    TAI_HOST_INTERFACE_ATTR_RX_RESET,

    /**
     * @brief TX Deserializer equalization LF_CTLE gain
     *
     * @type #tai_uint16_t
     */
    TAI_HOST_INTERFACE_ATTR_TX_DESERIAL_LF_CTLE_GAIN,

    /**
     * @brief TX Deserializer equalization CTLE gain
     *
     * @type #tai_uint16_t
     */
    TAI_HOST_INTERFACE_ATTR_TX_DESERIAL_CTLE_GAIN,

    /**
     * @brief TX Deserializer equalization DFE tap coefficient
     *
     * @type #tai_uint16_t
     */
    TAI_HOST_INTERFACE_ATTR_TX_DESERIAL_DFE_COEFFICIENT,

    /**
     * @brief RX Serializer Tap 0 gain
     *
     * @type #tai_uint16_t
     */
    TAI_HOST_INTERFACE_ATTR_RX_SERIAL_TAP0_GAIN,

    /**
     * @brief RX Serializer Tap 0 delay
     *
     * @type #tai_uint16_t
     */
    TAI_HOST_INTERFACE_ATTR_RX_SERIAL_TAP0_DELAY,

    /**
     * @brief RX Serializer Tap 1 gain
     *
     * @type #tai_uint16_t
     */
    TAI_HOST_INTERFACE_ATTR_RX_SERIAL_TAP1_GAIN,

    /**
     * @brief RX Serializer Tap 2 gain
     *
     * @type #tai_uint16_t
     */
    TAI_HOST_INTERFACE_ATTR_RX_SERIAL_TAP2_GAIN,

    /**
     * @brief RX Serializer Tap 2 delay
     *
     * @type #tai_uint16_t
     */
    TAI_HOST_INTERFACE_ATTR_RX_SERIAL_TAP2_DELAY

} tai_host_interface_ac400_custom_attr_t;

#define TAI_SYSLOG(lvl, ...)     tai_syslog(__TAI_MODULE__, lvl, __VA_ARGS__)
#define TAI_SYSLOG_DEBUG(...)    TAI_SYSLOG(TAI_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define TAI_SYSLOG_INFO(...)     TAI_SYSLOG(TAI_LOG_LEVEL_INFO, __VA_ARGS__)
#define TAI_SYSLOG_NOTICE(...)   TAI_SYSLOG(TAI_LOG_LEVEL_NOTICE, __VA_ARGS__)
#define TAI_SYSLOG_WARN(...)     TAI_SYSLOG(TAI_LOG_LEVEL_WARN, __VA_ARGS__)
#define TAI_SYSLOG_ERROR(...)    TAI_SYSLOG(TAI_LOG_LEVEL_ERROR, __VA_ARGS__)
#define TAI_SYSLOG_CRITICAL(...) TAI_SYSLOG(TAI_LOG_LEVEL_CRITICAL, __VA_ARGS__)

extern void tai_syslog(_In_ tai_api_t tai_api_id, _In_ tai_log_level_t log_level,
                       _In_ const char *format, ...) __attribute__ ((visibility ("hidden")));

extern const tai_attribute_value_t * find_attribute_in_list(
   _In_ tai_attr_id_t              attr_id,
   _In_ uint32_t                   attr_count,
   _In_ const tai_attribute_t     *attr_list) __attribute__ ((visibility ("hidden")));

extern tai_status_t convert_tai_error_to_list(_In_ tai_status_t err, _In_ uint32_t idx) __attribute__ ((visibility ("hidden")));

extern tai_status_t ac400_get_string(
   _In_    tai_object_id_t  module_id,
   _Inout_ tai_attribute_t *attr,
   _In_    uint16_t         reg_addr,
   _In_    uint32_t         str_len) __attribute__ ((visibility ("hidden")));

extern int ac400_get_transition_time(
   tai_network_interface_oper_status_t prevState,
   tai_network_interface_oper_status_t nextState) __attribute__ ((visibility ("hidden")));

extern tai_status_t ac400_get_module_oper_status(
   _In_ tai_object_id_t     module_id,
   _Inout_ tai_attribute_t *attr) __attribute__ ((visibility ("hidden")));

tai_status_t ac400_set_module_oper_status(
   _In_ tai_object_id_t  module_id,
   _In_ const tai_attribute_t *attr) __attribute__ ((visibility ("hidden")));

tai_status_t ac400_get_network_mode(
   _In_ tai_object_id_t     module_id,
   _Inout_ tai_attribute_t *attr) __attribute__ ((visibility ("hidden")));

extern tai_module_api_t            ac400_module_api __attribute__ ((visibility ("hidden")));
extern tai_host_interface_api_t    ac400_host_interface_api __attribute__ ((visibility ("hidden")));
extern tai_network_interface_api_t ac400_network_interface_api __attribute__ ((visibility ("hidden")));

#endif /* VOYAGER_TAI_ADAPTER_H__ */
