#ifndef _SUNNEED_LISTENER_H_
#define _SUNNEED_LISTENER_H_

#include <stdio.h>
#include <string.h>

#include <nng/nng.h>
#include <nng/protocol/reqrep0/req.h>
#include <nng/protocol/reqrep0/rep.h>

#include "log.h"

#define SUNNEED_LISTENER_URL "ipc:///tmp/sunneed.ipc"

int sunneed_listen(void);

#endif
