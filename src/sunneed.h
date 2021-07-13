#ifndef _SUNNEED_H_
#define _SUNNEED_H_

#include <stdbool.h>
#include <time.h>

#define APP_NAME "sunneed"

#ifdef LOG_PWR
    #define REQS_PER_LOG 10
    int last_capacity, curr_capacity, reqs_since_last_log;
    clock_t last_send, time_since_send;
#endif

typedef void* sunneed_worker_thread_result_t;

#endif
