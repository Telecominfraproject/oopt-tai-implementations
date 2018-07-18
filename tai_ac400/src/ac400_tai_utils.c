/**
 *  @file    ac400_tai_utils.c
 *  @brief   The TAI utility routines used by various TAI APIs
 *  @author  Scott Emery <scotte@cumulusnetworks.com>
 *
 *  @copyright Copyright (C) 2018 Cumulus Networks, Inc. All rights reserved
 *
 *  @remark  This source code is licensed under the BSD 3-Clause license found
 *           in the LICENSE file in the root directory of this source tree.
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include "voyager_tai_adapter.h"

#undef  __TAI_MODULE__
#define __TAI_MODULE__ TAI_API_UNSPECIFIED

/**
 * @brief Given a TAI logging level, convert to a syslog level.
 */
static int tai_to_syslog_level[TAI_LOG_LEVEL_MAX] = {
    [TAI_LOG_LEVEL_DEBUG]    = LOG_DEBUG,
    [TAI_LOG_LEVEL_INFO]     = LOG_INFO,
    [TAI_LOG_LEVEL_NOTICE]   = LOG_NOTICE,
    [TAI_LOG_LEVEL_WARN]     = LOG_WARNING,
    [TAI_LOG_LEVEL_ERROR]    = LOG_ERR,
    [TAI_LOG_LEVEL_CRITICAL] = LOG_CRIT
};

/**
 * @brief Given a TAI module, retrieve the syslog level for that module. This is
 *        set by calling the #tai_log_set function. The default is WARNING.
 */
static int api_log_level[TAI_API_MAX] = {
    [0 ... TAI_API_MAX-1] = LOG_WARNING
};

/**
 *  @brief Log a message to the syslog facility. The message may be filtered
 *         based on the TAI API's previously set logging level.
 *
 *  @param [in] tai_api_id The TAI API logging this message
 *  @param [in] log_level The TAI message priority
 *  @param [in] format A printf-like format string
 */
void tai_syslog(_In_ tai_api_t tai_api_id, _In_ tai_log_level_t log_level,
                _In_ const char *format, ...)
{
    int prevmask;
    va_list arglist;

    if ((TAI_API_UNSPECIFIED > tai_api_id) || (TAI_API_MAX <= tai_api_id)) {
        tai_api_id = TAI_API_UNSPECIFIED;
    }
    if ((TAI_LOG_LEVEL_DEBUG > log_level) || (TAI_LOG_LEVEL_MAX <= log_level)) {
        log_level = TAI_LOG_LEVEL_ERROR;
    }
    prevmask = setlogmask(LOG_UPTO(api_log_level[tai_api_id]));
    va_start(arglist, format);
    vsyslog(tai_to_syslog_level[log_level], format, arglist);
    va_end(arglist);
    setlogmask(prevmask);

    return;
}

/**
 * @brief Set log level for a tai api module. The default log level is
 *        TAI_LOG_WARN.
 *
 * @param [in] tai_api_id - TAI api ID
 * @param [in] log_level - log level
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
tai_status_t tai_log_set(_In_ tai_api_t tai_api_id,
                         _In_ tai_log_level_t log_level)
{
    if ((TAI_API_UNSPECIFIED > tai_api_id) || (TAI_API_MAX <= tai_api_id)) {
        TAI_SYSLOG_ERROR("Invalid API type %d", tai_api_id);
        return TAI_STATUS_INVALID_PARAMETER;
    }

    if ((TAI_LOG_LEVEL_DEBUG > log_level) || (TAI_LOG_LEVEL_MAX <= log_level)) {
        TAI_SYSLOG_ERROR("Invalid log level %d\n", log_level);
        return TAI_STATUS_INVALID_PARAMETER;
    }

    api_log_level[tai_api_id] = tai_to_syslog_level[log_level];
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Find an attribute in a list of attributes
 *
 * @param [in] attr_id The attribute ID to find
 * @param [in] attr_count The number of attributes in the list
 * @param [in] attr_list A list of attributes
 *
 * @return tai_attribute_value_t* A pointer to the attribute's value, or NULL if
 *         not found.
 */
const tai_attribute_value_t * find_attribute_in_list(
   _In_ tai_attr_id_t              attr_id,
   _In_ uint32_t                   attr_count,
   _In_ const tai_attribute_t     *attr_list)
{
    while (attr_count--) {
        if (attr_list->id == attr_id) {
            return &attr_list->value;
        }
        attr_list++;
    }
    return NULL;
}

/**
 * @brief Convert a TAI status code to an indexed status code
 *
 * If the status code in 'err' is one of the codes for a list of attributes, the
 * index value is added to that code. Otherwise the 'err' code is returned.
 *
 * @param err A TAI_STATUS_* code
 * @param idx An index into a list of attributes
 *
 * @return tai_status_t
 */
tai_status_t convert_tai_error_to_list( _In_ tai_status_t err, _In_ uint32_t idx)
{
    if (TAI_STATUS_INVALID_ATTRIBUTE_0    == err ||
        TAI_STATUS_INVALID_ATTR_VALUE_0   == err ||
        TAI_STATUS_ATTR_NOT_IMPLEMENTED_0 == err ||
        TAI_STATUS_UNKNOWN_ATTRIBUTE_0    == err ||
        TAI_STATUS_ATTR_NOT_SUPPORTED_0   == err) {
        return TAI_STATUS_CODE(TAI_STATUS_CODE(err) + idx);
    }
    return err;
}

/**
 * @brief Retrieve a string from the AC400 and store it in a character array
 *        attribute. Trailing spaces, if any, are removed.
 *
 * NOTE: A 16-byte string in the AC400 hardware requires a 17-byte character
 * array attribute, because an extra NULL byte is added at the end.
 *
 * @param module_id The module identifier of the AC400 module
 * @param attr The character array attribute
 * @param reg_addr The starting MDIO register address of the string
 * @param str_len The length of the string in the AC400 (does not include any
 *                trailing null)
 *
 * @return tai_status_t
 */
tai_status_t ac400_get_string(
   _In_    tai_object_id_t  module_id,
   _Inout_ tai_attribute_t *attr,
   _In_    uint16_t         reg_addr,
   _In_    uint32_t         str_len)
{
    char       *cp;
    char       *last_char;
    uint16_t    word1;

    /* Is there enough room in the attribute array? */
    if (attr->value.charlist.count <= str_len) {
        attr->value.charlist.count = str_len+1;
        return TAI_STATUS_BUFFER_OVERFLOW;
    }
    attr->value.charlist.count = str_len+1;
    cp = attr->value.charlist.list;
    last_char = cp-1;

    while (str_len--) {
        if (ac400_mdio_read(module_id, reg_addr++, &word1)) {
            return TAI_STATUS_FAILURE;
        }
        if ((word1 & 0x00FF) != ' ') {
            last_char = cp;
        }
        *cp++ = word1 & 0x00FF;
    }
    *(last_char+1) = 0;
    return TAI_STATUS_SUCCESS;
}

/**
 *
 * @brief Retrieves the maximum amount of time, in seconds, it takes to
 *        transition between two AC400 states.
 *
 * NOTE: These values were obtained from the AC400 module registers which
 * specify the maximum amount of time which transitory states can take.
 *
 * @param [in] prevState The previous state of the network interface
 * @param [in] nextState The next state of the network interface
 *
 * @return The time, in seconds.
 */
int ac400_get_transition_time(
   tai_network_interface_oper_status_t prevState,
   tai_network_interface_oper_status_t nextState)
{
    /* This 2D array provides the maximum amount of time it takes to
       transition from one module state to another, through the shortest path.
       The rows are the previous state and the columns are the new state.
       Entries are time, in seconds. Since we can only transition to four new
       states, only four columns are filled out. */
    int transTime[TAI_NETWORK_INTERFACE_OPER_STATUS_MAX][TAI_NETWORK_INTERFACE_OPER_STATUS_MAX] = {
    /*                                                                      New state
                                                                                             T   H
                                                                          L          T       x   i
                                                               U          o  H       x       T   P
                                                               n          w  i       T       u   w
                                                               k   R      P  P   T   u   R   r   r  F
                                                               n   e  I   o  w   x   r   e   n   D  a
                                                               o   s  n   w  r   O   n   a   O   o  u
                                                               w   e  i   e  U   f   O   d   f   w  l
                     Previous State                            n   t  t   r  p   f   n   y   f   n  t */
    /*  TAI_NETWORK_INTERFACE_OPER_STATUS_UNKNOWN         */ { 0,  0, 0,  0, 0,  0,  0,  0,  0,  0, 0 },
    /*  TAI_NETWORK_INTERFACE_OPER_STATUS_RESET           */ { 0,  0, 0, 20,20,200,200,201,201,200, 0 },
    /*  TAI_NETWORK_INTERFACE_OPER_STATUS_INITIALIZE      */ { 0,  0, 0, 20,20,200,200,201,201,200, 0 },
    /*  TAI_NETWORK_INTERFACE_OPER_STATUS_LOW_POWER       */ { 0,  0, 0,  0, 0,180,180,181,181,180, 0 },
    /*  TAI_NETWORK_INTERFACE_OPER_STATUS_HIGH_POWER_UP   */ { 0,  0, 0,190, 0,180,180,181,181,180, 0 },
    /*  TAI_NETWORK_INTERFACE_OPER_STATUS_TX_OFF          */ { 0,  0, 0, 10,10,  0,  0,  1,  1,  0, 0 },
    /*  TAI_NETWORK_INTERFACE_OPER_STATUS_TX_TURN_ON      */ { 0,  0, 0, 12,12,  2,  0,  1,  1,  2, 0 },
    /*  TAI_NETWORK_INTERFACE_OPER_STATUS_READY           */ { 0,  0, 0, 11,11,  1,  1,  0,  0,  1, 0 },
    /*  TAI_NETWORK_INTERFACE_OPER_STATUS_TX_TURN_OFF     */ { 0,  0, 0, 11,11,  1,  1,  2,  0,  1, 0 },
    /*  TAI_NETWORK_INTERFACE_OPER_STATUS_HIGH_POWER_DOWN */ { 0,  0, 0, 10,10,190,190,191,191,  0, 0 },
    /*  TAI_NETWORK_INTERFACE_OPER_STATUS_FAULT           */ { 0,  0, 0,  0, 0,  0,  0,  0,  0,  0, 0 }
    };

    /* Are the states valid? */
    if ((TAI_NETWORK_INTERFACE_OPER_STATUS_MAX < prevState) ||
        (TAI_NETWORK_INTERFACE_OPER_STATUS_MAX < nextState)) {
        return 0;
    }

    return transTime[prevState][nextState];
}
