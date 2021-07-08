#ifndef _SUNNEED_H_
#define _SUNNEED_H_

#include <stdbool.h>

#define APP_NAME "sunneed"

#ifdef LOG_PWR
    #define REQS_PER_LOG 10
    int last_capacity, curr_capacity, reqs_since_last_log;
    double last_send, time_since_send;
#endif

typedef void* sunneed_worker_thread_result_t;

#endif
