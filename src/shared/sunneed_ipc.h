#ifndef _SUNNEED_IPC_H_
#define _SUNNEED_IPC_H_

#include <stdio.h>
#include <stdlib.h>

#define SUNNEED_LISTENER_URL "ipc:///tmp/sunneed.ipc"

void (*_sunneed_nng_error_func)(const char *nng_call_name, int rv);

/**
 * Sets the function to be called when an error is encountered during a SUNNEED_NNG_TRY_* macro.
 * This function takes two arguments: a string representing the name of the function, and an integer containing the
 *  return value of the failed NNG function call.
 * Usually this function should somehow call `nng_strerror` to report NNG's description of the error.
 */
#define SUNNEED_NNG_SET_ERROR_REPORT_FUNC(FUNCNAME) _sunneed_nng_error_func = FUNCNAME;

/** Try to perform an NNG function, expanding to returning 1 if the NNG call fails. */
#define SUNNEED_NNG_TRY_RET(CALL, ERROR_COND, ARGS...)          \
    {                                                           \
        int _rv;                                                \
        SUNNEED_NNG_TRY_RET_SET(CALL, _rv, ERROR_COND, ##ARGS); \
    }

/**
 * Try to perform an NNG function, expanding to returning 1 if the NNG call fails.
 * In the process of calling the function, the variable TARGET will be assigned its return value.
 */
#define SUNNEED_NNG_TRY_RET_SET(CALL, TARGET, ERROR_COND, ARGS...) \
    { _SUNNEED_NNG_TRY_CONTAINER(CALL, TARGET, ERROR_COND, return 1, ##ARGS); }

/** Try to perform an NNG function. */
#define SUNNEED_NNG_TRY(CALL, ERROR_COND, ARGS...)          \
    {                                                       \
        int _rv;                                            \
        SUNNEED_NNG_TRY_SET(CALL, _rv, ERROR_COND, ##ARGS); \
    }

/**
 * Try to perform an NNG function.
 * In the process of calling the function, the variable TARGET will be assigned its return value.
 */
#define SUNNEED_NNG_TRY_SET(CALL, TARGET, ERROR_COND, ARGS...) \
    { _SUNNEED_NNG_TRY_CONTAINER(CALL, TARGET, ERROR_COND, , ##ARGS); }

#ifdef SUNNEED_BUILD_TYPE_PRODUCTION
#define _SUNNEED_NNG_TRY_CONTAINER(CALL, TARGET, ERROR_COND, ON_ERROR, ARGS...) \
    {                                                                           \
        if ((TARGET = CALL(ARGS)) ERROR_COND) {                                 \
            _sunneed_nng_error_func(#CALL, TARGET);                             \
            ON_ERROR;                                                           \
        }                                                                       \
    }
#else
#define _SUNNEED_NNG_TRY_CONTAINER(CALL, TARGET, ERROR_COND, ON_ERROR, ARGS...)         \
    {                                                                                   \
        if ((TARGET = CALL(ARGS)) ERROR_COND) {                                         \
            if (!_sunneed_nng_error_func) {                                             \
                fprintf(stderr,                                                         \
                        "An NNG error was encountered but there is no error handler.\n" \
                        "Error details: " #CALL ": %s\n",                               \
                        nng_strerror(TARGET));                                          \
                exit(1);                                                                \
            }                                                                           \
            _sunneed_nng_error_func(#CALL, TARGET);                                     \
            ON_ERROR;                                                                   \
        }                                                                               \
    }
#endif

#define SUNNEED_NNG_MSG_LEN_FIX(LEN_VAR) \
    if ((LEN_VAR / 2) % 2 == 1) \
        LEN_VAR++;


/****
 * IPC helpers
 ***/

int
sunneed_ipc_register_self(void);

#endif
