#ifndef _SUNNEED_IPC_H_
#define _SUNNEED_IPC_H_

#define SUNNEED_LISTENER_URL "ipc:///tmp/sunneed.ipc"

#define SUNNEED_IPC_TEST_REQ_STR "REQ"
#define SUNNEED_IPC_TEST_REP_STR "REP"

#define SUNNEED_IPC_REQ_REGISTER "HELLOTHERE"
#define SUNNEED_IPC_REQ_UNREGISTER "SEEYA"

#define SUNNEED_IPC_REQ_GET_DEVICE_HANDLE "HANDLE"

#define SUNNEED_IPC_REP_SUCCESS "YEAH"
#define SUNNEED_IPC_REP_RESULT "RESULT"
#define SUNNEED_IPC_REP_FAILURE "SORRY"

void (*_sunneed_nng_error_func)(const char *nng_call_name, int rv);

/** 
 * Sets the function to be called when an error is encountered during a SUNNEED_NNG_TRY_* macro.
 * This function takes two arguments: a string representing the name of the function, and an integer containing the
 *  return value of the failed NNG function call.
 * Usually this function should somehow call `nng_strerror` to report NNG's description of the error.
 */
#define SUNNEED_NNG_SET_ERROR_REPORT_FUNC(FUNCNAME) \
    _sunneed_nng_error_func = FUNCNAME;

/** Try to perform an NNG function, expanding to returning 1 if the NNG call fails. */
#define SUNNEED_NNG_TRY_RET(CALL, ERROR_COND, ARGS...) { \
    int _rv; \
    SUNNEED_NNG_TRY_RET_SET(CALL, _rv, ERROR_COND, ## ARGS); \
}

/** 
 * Try to perform an NNG function, expanding to returning 1 if the NNG call fails. 
 * In the process of calling the function, the variable TARGET will be assigned its return value.
 */
#define SUNNEED_NNG_TRY_RET_SET(CALL, TARGET, ERROR_COND, ARGS...) { \
    if ((TARGET = CALL(ARGS)) ERROR_COND) { \
        _sunneed_nng_error_func(#CALL, TARGET); \
        return 1; \
    } \
}

/** Try to perform an NNG function. */
#define SUNNEED_NNG_TRY(CALL, ERROR_COND, ARGS...) { \
    int _rv; \
    SUNNEED_NNG_TRY_SET(CALL, _rv, ERROR_COND, ## ARGS); \
}
    
/**
 * Try to perform an NNG function.
 * In the process of calling the function, the variable TARGET will be assigned its return value.
 */
#define SUNNEED_NNG_TRY_SET(CALL, TARGET, ERROR_COND, ARGS...) { \
    if ((TARGET = CALL(ARGS)) ERROR_COND) { \
        _sunneed_nng_error_func(#CALL, TARGET); \
    } \
}

#endif
