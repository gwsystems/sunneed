#ifndef _SUNNEED_PROC_H_
#define _SUNNEED_PROC_H_

#include <sys/types.h>

#include "sunneed.h"

#define MAX_TENANTS 2

typedef unsigned int sunneed_tenant_id_t;

// TODO Separate tenants from processes.
struct sunneed_tenant {
    sunneed_tenant_id_t id;
    pid_t pid;
    float power_proportion;
    bool is_active;
};

int
sunneed_init_tenants(void);

unsigned int
sunneed_get_num_tenants(void);

#endif
