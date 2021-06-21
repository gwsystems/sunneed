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

#define PASSIVE_PWR_PER_SEC 0.07667 

#define PASSIVE_PWR_PER_MIN PASSIVE_PWR_PER_SEC * 60

#ifdef LOG_PWR
#define REQUESTS_PER_PWR_LOG 4
#endif

// TODO This is waaaaaaaaaaaaaaaaaaaaay too big.
#define QUANTUM_DURATION_MS 5000

struct sunneed_power_usage_event {
    struct {
        // The moment the power event occurred.
        struct timespec timestamp;

        struct sunneed_tenant *tenant;

        // If NULL, then the power event is a CPU usage digest.
        struct sunneed_device *device;

        // Unused for now, can configure parameters of the device interaction.
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

sunneed_worker_thread_result_t
sunneed_quantum_worker(void *args);

struct sunneed_power_usage_event *power_usage_evs;

struct sunneed_device devices[MAX_DEVICES];

#ifdef LOG_PWR
clock_t last_pwr_log_t, curr_pwr_log_t;
int requests_since_last_log, last_logged_pwr;
int requests_arr_dirty[REQUESTS_PER_PWR_LOG]; /* holds requests intercepted by sunneed until REQUESTS_PER_PWR_LOG requests hit, then will be flushed to logfile */
#endif
#endif
