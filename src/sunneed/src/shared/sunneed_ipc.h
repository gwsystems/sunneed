#ifndef _SUNNEED_IPC_H_
#define _SUNNEED_IPC_H_

#define SUNNEED_LISTENER_URL "ipc:///tmp/sunneed.ipc"

#define SUNNEED_IPC_TEST_REQ_STR "REQ"
#define SUNNEED_IPC_TEST_REP_STR "REP"

#define SUNNEED_IPC_REQ_GET_DEVICE_HANDLE "HANDLE"

#define SUNNEED_IPC_REP_STATE_SUCCESS "YEAH"
#define SUNNEED_IPC_REP_STATE_FAILURE "SORRY"

void (*_sunneed_nng_error_func)(const char *nng_call_name, int rv);

#define SUNNEED_NNG_SET_ERROR_REPORT_FUNC(FUNCNAME) \
    _sunneed_nng_error_func = FUNCNAME;

#define SUNNEED_NNG_TRY_RET(CALL, ERROR_COND, ARGS...) { \
    int _rv; \
    SUNNEED_NNG_TRY_RET_SET(CALL, _rv, ERROR_COND, ## ARGS); \
}

#define SUNNEED_NNG_TRY_RET_SET(CALL, TARGET, ERROR_COND, ARGS...) { \
    if ((TARGET = CALL(ARGS)) ERROR_COND) { \
        _sunneed_nng_error_func(#CALL, TARGET); \
        return 1; \
    } \
}

#define SUNNEED_NNG_TRY(CALL, ERROR_COND, ARGS...) { \
    int _rv; \
    SUNNEED_NNG_TRY_SET(CALL, _rv, ERROR_COND, ## ARGS); \
}
    
#define SUNNEED_NNG_TRY_SET(CALL, TARGET, ERROR_COND, ARGS...) { \
    if ((TARGET = CALL(ARGS)) ERROR_COND) { \
        _sunneed_nng_error_func(#CALL, TARGET); \
    } \
}



#endif
