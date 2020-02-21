#ifndef _SUNNEED_LISTENER_H_
#define _SUNNEED_LISTENER_H_

#include <stdio.h>
#include <string.h>

#include <nng/nng.h>
#include <nng/protocol/reqrep0/req.h>
#include <nng/protocol/reqrep0/rep.h>

#include "log.h"

#define SUNNEED_LISTENER_URL "ipc:///tmp/sunneed.ipc"

#define SUNNEED_IPC_TEST_REQ_STR "REQ"
#define SUNNEED_IPC_TEST_REP_STR "REP"

int sunneed_listen(void);

#endif
