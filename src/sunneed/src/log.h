#ifndef _LOG_H_
#define _LOG_H_

/* 
 * Defines a variety of macros to be used for logging information.
 * Assign a FILE* to `logfile` during runtime to redirect the log to that file.
 */

#include <stdio.h>
#include <time.h>

FILE *logfile;

#define LOGL_DEBUG 'D'
#define LOGL_INFO 'I'
#define LOGL_WARN 'W'
#define LOGL_ERROR 'E'

#define LOG(LEVEL, MESSAGE, ...) \
    { \
        FILE *_logfile = logfile; \
        if (!logfile) { \
            _logfile = stdout; \
        } \
        time_t _now = time(NULL); \
        struct tm *_time = localtime(&_now); \
        char _time_str[21]; \
        strftime(_time_str, 21, "%Y-%m-%d %H:%M:%S", _time); \
        fprintf(_logfile, "%c[%s] " MESSAGE "\n", LEVEL, _time_str, ## __VA_ARGS__); \
    }

#define LOG_D(MESSAGE, ...) \
    LOG(LOGL_DEBUG, MESSAGE, ## __VA_ARGS__);
#define LOG_I(MESSAGE, ...) \
    LOG(LOGL_INFO, MESSAGE, ## __VA_ARGS__);
#define LOG_W(MESSAGE, ...) \
    LOG(LOGL_WARN, MESSAGE, ## __VA_ARGS__);
#define LOG_E(MESSAGE, ...) \
    LOG(LOGL_ERROR, MESSAGE, ## __VA_ARGS__);

#endif
