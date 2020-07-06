#ifndef _SUNNEED_POWER_H_
#define _SUNNEED_POWER_H_

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "log.h"
#include "shared/sunneed_pip_interface.h"
#include "sunneed.h"
#include "sunneed_device.h"
#include "sunneed_proc.h"

#define MAX_DEVICES 64

#define QUANTUMS_RINGBUF_SZ 16

struct sunneed_power_usage_event {
    struct {
        struct timespec timestamp;
        struct sunneed_tenant *tenant;
        struct sunneed_device *device;
        void *args;
    } ev;
    struct sunneed_power_usage_event *next;
};

int
sunneed_record_power_usage_event(struct sunneed_power_usage_event ev);
int
sunneed_quantum_begin(void);
int
sunneed_quantum_end(void);

struct sunneed_power_usage_event *power_usage_evs;

struct sunneed_device devices[MAX_DEVICES];

#endif
