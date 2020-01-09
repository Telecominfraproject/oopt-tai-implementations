#ifndef __TAI_MUX_NETWORKIF__
#define __TAI_MUX_NETWORKIF__

#include <tai.h>

typedef enum _mux_network_interface_attr_t
{
    /**
     * @brief The real OID
     *
     * @type #tai_object_id_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_MUX_REAL_OID = TAI_NETWORK_INTERFACE_ATTR_CUSTOM_MUX_START,

} mux_network_interface_attr_t;

#endif
