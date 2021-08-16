#ifndef _SUNNEED_LISTENER_H_
#define _SUNNEED_LISTENER_H_

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/reqrep0/req.h>

#include "log.h"
#include "shared/sunneed_ipc.h"
#include "sunneed.h"
#include "sunneed_power.h"
#include "sunneed_proc.h"
#include "sunneed_queued_requests.h"

#define SUNNEED_MESSAGE_DEFAULT_BODY_SZ 64
#define SUNNEED_MAX_IPC_CLIENTS 512
#define SUNNEED_DEVICE_PATH_MAX_LEN 64

#ifdef LOG_PWR

#define LOGS_PER_PWR_RECORDING 20

int current_log_dirty[LOGS_PER_PWR_RECORDING]; /* holds log of requests before LOGS_PER_PWR_RECORDING requests made and requests are written out to logfile along with power recording */
int last_logged_pwr, reqs_since_log_flush;
#endif

int
sunneed_listen(void);

sunneed_worker_thread_result_t
sunneed_request_servicer(__attribute__((unused)) void *args);
#endif
