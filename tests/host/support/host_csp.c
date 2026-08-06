/* Imported from C:\PSC\csp-rs485\tests\host (authorized subset). */
/* This import makes no assertion about upstream licensing. */
#include "host_csp.h"

#include <csp/csp.h>

int host_csp_init(void)
{
    csp_conf_t config;

    csp_conf_get_defaults(&config);
    config.address = 10U;
    config.buffers = 20U;
    config.buffer_data_size = 300U;

    return csp_init(&config);
}

void host_csp_cleanup(void)
{
    csp_free_resources();
}
