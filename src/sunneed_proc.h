#ifndef _SUNNEED_PROC_H_
#define _SUNNEED_PROC_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <wait.h>

#include "log.h"
#include "sunneed.h"

#define MAX_TENANTS 4

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

enum battery_state {
    FULL,
    CHARGING,
    DISCHARGING,
    HALF,
    LOW,
    EMPTY
};

int sunneed_stepper_driver_pid, sunneed_camera_driver_pid;

int
sunneed_update_tenant_cpu_usage(void);

int
sunneed_init_tenants(void);

struct sunneed_tenant *
sunneed_tenant_register(pid_t pid);

int
sunneed_tenant_unregister(struct sunneed_tenant *tenant);

unsigned int
sunneed_get_num_tenants(void);

int
sunneed_get_tenant_cpu_usage(sunneed_tenant_id_t tenant_id);

sunneed_worker_thread_result_t
sunneed_proc_monitor(void *args);

sunneed_worker_thread_result_t
sunneed_stepperMotor_driver(void *args);

sunneed_worker_thread_result_t
sunneed_camera_driver(void *args);

#endif
