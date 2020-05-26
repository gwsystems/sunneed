#ifndef _SUNNEED_PROC_H_
#define _SUNNEED_PROC_H_

#include <sys/types.h>

#include "sunneed.h"
#include "log.h"
#include <unistd.h>

#define MAX_TENANTS 2

typedef unsigned int sunneed_tenant_id_t;

// TODO Separate tenants from processes.
struct sunneed_tenant {
    sunneed_tenant_id_t id;
    pid_t pid;
    float power_proportion;
    bool is_active;
};

struct tenant_cpu_usage {
    unsigned long long user, sys;
};

struct {
    unsigned long long user, nice, sys, idle;
    struct tenant_cpu_usage tenants[MAX_TENANTS];
} cpu_usage;

int
sunneed_update_tenant_cpu_usage(void);

int
sunneed_init_tenants(void);

int
sunneed_tenant_register(sunneed_tenant_id_t tenant_id, pid_t pid);

unsigned int
sunneed_get_num_tenants(void);

int
sunneed_get_tenant_cpu_usage(sunneed_tenant_id_t tenant_id);

void
*sunneed_proc_monitor(void *args);

#endif
