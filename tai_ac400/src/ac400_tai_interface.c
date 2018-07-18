/**
 *  @file    ac400_tai_interface.c
 *  @brief   The main TAI interface routines
 *  @author  Scott Emery <scotte@cumulusnetworks.com>
 *
 *  @copyright Copyright (C) 2018 Cumulus Networks, Inc. All rights reserved
 *
 *  @remark  This source code is licensed under the BSD 3-Clause license found
 *           in the LICENSE file in the root directory of this source tree.
 */

#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <pthread.h>
#include <unistd.h>
#include "voyager_tai_adapter.h"

#undef  __TAI_MODULE__
#define __TAI_MODULE__ TAI_API_UNSPECIFIED

static tai_service_method_table_t adapter_host_fns;
static bool                       initialized = false;
static pthread_t                  module_presence_thread;

/**
 * @brief   A separate thread which monitors the presence of modules and alerts
 *          the adapter when the state changes.
 *
 * @param   [in, out] param Not used
 */
void *module_presence(void *param)
{
    bool absent[VOYAGER_NUM_AC400];
    ac400_object_id_t ac400_obj;
    tai_object_id_t *module_id = (tai_object_id_t *) &ac400_obj;
    int mod;
    bool mod_abs;
    char location[2];

    for (mod=0; mod < VOYAGER_NUM_AC400; mod++) {
        absent[mod] = true;
    }

    ac400_obj.type = TAI_API_MODULE;
    while (initialized) {
        for (mod=0; mod < VOYAGER_NUM_AC400; mod++) {
            ac400_obj.value = mod+1;
            if (!ac400_get_mod_abs(*module_id, &mod_abs)) {
                if (absent[mod] != mod_abs) {
                    if (NULL != adapter_host_fns.module_presence) {
                        TAI_SYSLOG_DEBUG("Module %d is now %s", mod+1, mod_abs ? "absent" : "present");
                        snprintf(location, 2, "%d", mod+1);
                        adapter_host_fns.module_presence(!mod_abs, location);
                    }
                    absent[mod] = mod_abs;
                }
            }
        }
        usleep(500000);
    }
    pthread_exit(NULL);
}

/**
 *  @brief  Adapter module initialization call. This is NOT for SDK
 *          initialization.
 *
 *  @param [in] flags Reserved for future use, must be zero
 *  @param [in] services Methods table with services provided by adapter host
 *
 *  @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
tai_status_t tai_api_initialize(_In_ uint64_t flags,
                                _In_ const tai_service_method_table_t* services)
{
    int ret;

    if (0 != flags) {
        TAI_SYSLOG_ERROR("Invalid flags passed to TAI API initialize");
        return TAI_STATUS_INVALID_PARAMETER;
    }

    if (NULL == services) {
        TAI_SYSLOG_ERROR("Invalid services handle passed to TAI API initialize");
        return TAI_STATUS_INVALID_PARAMETER;
    }

    memcpy(&adapter_host_fns, services, sizeof(adapter_host_fns));
    /**
     * @todo Add initialization calls for the APIs
     */
    initialized = true;

    ret = pthread_create(&module_presence_thread, NULL, module_presence, NULL);
    if (ret) {
        TAI_SYSLOG_ERROR("Unable to create module presence thread: %d", ret);
    }
    return TAI_STATUS_SUCCESS;
}

/**
 *  @brief  Retrieve a pointer to the C-style method table for desired TAI
 *          functionality as specified by the given tai_api_id.
 *
 *  @param [in] tai_api_id TAI api ID
 *  @param [out] api_method_table Caller allocated method table. The table must
 *               remain valid until the tai_api_uninitialize() is called.
 *  @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
tai_status_t tai_api_query(_In_ tai_api_t tai_api_id,
                           _Out_ void** api_method_table)
{
    if (!initialized) {
        TAI_SYSLOG_ERROR("TAI API not initialized before calling API query");
        return TAI_STATUS_UNINITIALIZED;
    }
    if (NULL == api_method_table) {
        TAI_SYSLOG_ERROR("NULL method table passed to TAI API initialize");
        return TAI_STATUS_INVALID_PARAMETER;
    }

    switch (tai_api_id) {
        case TAI_API_MODULE:
            *(const tai_module_api_t**)api_method_table = &ac400_module_api;
            return TAI_STATUS_SUCCESS;

        case TAI_API_HOSTIF:
            *(const tai_host_interface_api_t**)api_method_table =
                &ac400_host_interface_api;
            return TAI_STATUS_SUCCESS;

        case TAI_API_NETWORKIF:
            *(const tai_network_interface_api_t**)api_method_table =
                &ac400_network_interface_api;
            return TAI_STATUS_SUCCESS;

        default:
            TAI_SYSLOG_ERROR("Invalid API type %d", tai_api_id);
            return TAI_STATUS_INVALID_PARAMETER;
    }
}

/**
 *  @brief  Uninitialization of the adapter module. TAI functionalities,
 *          retrieved via tai_api_query() cannot be used after this call.
 *
 *  @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
tai_status_t tai_api_uninitialize(void)
{
    initialized = false;
    pthread_join(module_presence_thread, NULL);
    memset(&adapter_host_fns, 0, sizeof(adapter_host_fns));

    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Query tai object type.
 *
 * @param [in] tai_object_id
 *
 * @return Return #TAI_OBJECT_TYPE_NULL when tai_object_id is not valid.
 *         Otherwise, return a valid tai object type TAI_OBJECT_TYPE_XXX
 */
tai_object_type_t tai_object_type_query(_In_ tai_object_id_t tai_object_id)
{
    tai_object_type_t type = ((ac400_object_id_t*)&tai_object_id)->type;

    if (TAI_OBJECT_TYPE_MAX > type) {
        return type;
    } else {
        TAI_SYSLOG_ERROR("Unknown type %d", type);
        return TAI_OBJECT_TYPE_NULL;
    }
}
