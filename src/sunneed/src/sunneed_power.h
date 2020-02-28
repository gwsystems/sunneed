#ifndef _SUNNEED_POWER_H_
#define _SUNNEED_POWER_H_

#include <time.h>
#include <string.h>
#include <stdlib.h>

#include "sunneed.h"
#include "log.h"
#include "sunneed_proc.h"
#include "sunneed_pip.h"

#define QUANTUMS_RINGBUF_SZ 16

struct sunneed_device {
    // TODO Implement me.
    double power_consumption;
};

struct sunneed_power_usage_event {
    struct {
        struct timespec timestamp;
        struct sunneed_tenant *tenant;
        struct sunneed_device *device;
        void *args;
    } ev;
    struct sunneed_power_usage_event *next;
};

int sunneed_record_power_usage_event(struct sunneed_power_usage_event ev);
int sunneed_quantum_begin(void);
int sunneed_quantum_end(void);

struct sunneed_power_usage_event *power_usage_evs;

#endif
