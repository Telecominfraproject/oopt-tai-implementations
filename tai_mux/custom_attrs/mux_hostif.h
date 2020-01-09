#ifndef __TAI_MUX_HOSTIF__
#define __TAI_MUX_HOSTIF__

#include <tai.h>

typedef enum _mux_host_interface_attr_t
{
    /**
     * @brief The real OID
     *
     * @type #tai_object_id_t
     * @flags READ_ONLY
     */
    TAI_HOST_INTERFACE_ATTR_MUX_REAL_OID = TAI_HOST_INTERFACE_ATTR_CUSTOM_MUX_START,

} mux_host_interface_attr_t;

#endif
