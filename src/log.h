#ifndef _LOG_H_
#define _LOG_H_

/*
 * Defines a variety of macros to be used for logging information.
 * Assign a FILE* to `logfile` during runtime to redirect the log to that file.
 */

#include <stdio.h>
#include <time.h>

FILE *logfile, *logfile_pwr;

#define LOGL_DEBUG "D\e[38;5;240m"
#define LOGL_INFO "I"
#define LOGL_WARN "W\e[0;33m"
#define LOGL_ERROR "E\e[0;31m"


#define LOG_PWR_EVENT(LEVEL, MESSAGE, ...)                                                \
    {                                                                               \
        FILE *_logfile = logfile_pwr;                                               \
        if (logfile_pwr) {							    \
		_logfile = fopen("sunneed_pwr_log.txt", "w+");			    \
	}									    \
	time_t _now = time(NULL);                                                   \
        struct tm *_time = localtime(&_now);                                        \
        char _time_str[21];                                                         \
        strftime(_time_str, 21, "%Y-%m-%d %H:%M:%S", _time);                        \
        fprintf(_logfile, "%s[%s] " MESSAGE "\e[0m\n", LEVEL, _time_str, ##__VA_ARGS__); \
	fflush(_logfile);							    \
    }

#define LOG(LEVEL, MESSAGE, ...)                                                    \
    {                                                                               \
        FILE *_logfile = logfile;                                                   \
        if (!logfile) {                                                             \
            _logfile = stdout;                                                      \
        }                                                                           \
        time_t _now = time(NULL);                                                   \
        struct tm *_time = localtime(&_now);                                        \
        char _time_str[21];                                                         \
        strftime(_time_str, 21, "%Y-%m-%d %H:%M:%S", _time);                        \
        fprintf(_logfile, "%s[%s] " MESSAGE "\e[0m\n", LEVEL, _time_str, ##__VA_ARGS__); \
    }

#define LOG_D(MESSAGE, ...) LOG(LOGL_DEBUG, MESSAGE, ##__VA_ARGS__);
#define LOG_I(MESSAGE, ...) LOG(LOGL_INFO, MESSAGE, ##__VA_ARGS__);
#define LOG_W(MESSAGE, ...) LOG(LOGL_WARN, MESSAGE, ##__VA_ARGS__);
#define LOG_E(MESSAGE, ...) LOG(LOGL_ERROR, MESSAGE, ##__VA_ARGS__);
#define LOG_P(MESSAGE, ...) LOG_PWR_EVENT(LOGL_INFO, MESSAGE, ##__VA_ARGS__);
#endif
