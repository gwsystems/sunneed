#ifndef _SUNNEED_DEVICE_H_
#define _SUNNEED_DEVICE_H_

#include "sunneed.h"
#include "log.h"

#include "shared/sunneed_device_type.h"
#include "protobuf/c/device.pb-c.h"

#include <stdio.h>
#include <string.h>

#define MAX_DEVICES 64

struct sunneed_device {
    void *dlhandle;
    int handle;
    char *identifier;
    void *(*get)(void *);
    double (*power_consumption)(void *);
    bool is_linked;

    enum sunneed_device_type device_type_kind;
    union {
        struct sunneed_device_type_file_lock file_lock;
    } device_type;
};

void *sunneed_device_get_device_type(struct sunneed_device *device);

struct sunneed_device *
sunneed_device_file_is_locked(const char *pathname);

extern struct sunneed_device devices[];

#endif
