#ifndef _SUNNEED_LISTENER_H_
#define _SUNNEED_LISTENER_H_

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>

#include <nng/nng.h>
#include <nng/protocol/reqrep0/req.h>
#include <nng/protocol/reqrep0/rep.h>

#include "sunneed.h"
#include "sunneed_power.h"
#include "shared/sunneed_ipc.h"

#include "log.h"

#define SUNNEED_MESSAGE_DEFAULT_BODY_SZ 64
#define SUNNEED_MAX_IPC_CLIENTS 512
#define SUNNEED_DEVICE_PATH_MAX_LEN 64

enum sunneed_client_ipc_state {
    STATE_NONE,
    STATE_GET_HANDLE
};

int sunneed_listen(void);

#endif
