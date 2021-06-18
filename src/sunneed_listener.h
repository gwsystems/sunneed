#ifndef _SUNNEED_LISTENER_H_
#define _SUNNEED_LISTENER_H_

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
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

#define SUNNEED_MESSAGE_DEFAULT_BODY_SZ 64
#define SUNNEED_MAX_IPC_CLIENTS 512
#define SUNNEED_DEVICE_PATH_MAX_LEN 64

int
sunneed_listen(void);

#endif
