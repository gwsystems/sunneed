#ifndef _SUNNEED_DEVICE_H_
#define _SUNNEED_DEVICE_H_

#include "sunneed.h"
#include "log.h"

#include "shared/sunneed_device_type.h"
#include "protobuf/c/device.pb-c.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DEVICES 64

#define MAX_LOCKED_FILES 1024

#define SUNNEED_DEVICE_FLAG_SILENT_FAIL (1 << 0)

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
    } device_type_data;
};

bool
sunneed_device_is_linked(struct sunneed_device *device);

struct sunneed_device *
sunneed_device_file_locker(const char *pathname);

char *
sunneed_device_get_dummy_file(const char *orig_path);

extern struct sunneed_device devices[];

#endif
