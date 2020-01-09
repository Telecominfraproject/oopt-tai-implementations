#ifndef __TAI_MUX_MODULE__
#define __TAI_MUX_MODULE__

#include <tai.h>

typedef enum _tai_mux_platform_adapter_type_t
{
    TAI_MUX_PLATFORM_ADAPTER_TYPE_UNKNOWN,
    TAI_MUX_PLATFORM_ADAPTER_TYPE_STATIC,
    TAI_MUX_PLATFORM_ADAPTER_TYPE_MAX,
} tai_mux_platform_adapter_type_t;

typedef enum _mux_module_attr_t
{
    /**
     * @brief The platform adapter type
     *
     * @type #tai_mux_platform_adapter_type_t
     * @flags READ_ONLY
     */
    TAI_MODULE_ATTR_MUX_PLATFORM_ADAPTER_TYPE = TAI_MODULE_ATTR_CUSTOM_MUX_START,

    /**
     * @brief The loaded TAI library
     *
     * @type #tai_char_list_t
     * @flags READ_ONLY
     */
    TAI_MODULE_ATTR_MUX_CURRENT_LOADED_TAI_LIBRARY,

    /**
     * @brief The real OID
     *
     * @type #tai_object_id_t
     * @flags READ_ONLY
     */
    TAI_MODULE_ATTR_MUX_REAL_OID,

} mux_module_attr_t;

#endif
