#ifndef _SUNNEED_DEVICE_H_
#define _SUNNEED_DEVICE_H_

#include "sunneed.h"
#include "log.h"

#include "shared/sunneed_device_type.h"
#include "shared/sunneed_files.h"
#include "protobuf/c/device.pb-c.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEVICE_IDENTIFIER_LEN 32

#define MAX_DEVICES 64

#define DUMMY_FILE_PATH_LEN 128

#define SUNNEED_DEVICE_FLAG_SILENT_FAIL (1 << 0)

struct sunneed_device {
    /** Set to true when all data has been prepared in this struct. */
    bool is_ready;

    /** Numerical identifier of the device. */
    int handle;
    
    /** Human-readable name for the device (e.g. "camera"). */
    char *identifier;

    /** Specifices the union member to write to in `device_type_data`. */
    enum sunneed_device_type device_type_kind;

    /** One of many different structs representing arbitrary data of the device interface. */
    union {
        struct sunneed_device_type_file_lock *file_lock;
    } device_type_data;
};

typedef enum {
    CLOCKWISE,
    COUNTER_CLOCKWISE,
    STOPPED
} stepperMotor_Direction;

struct sunneed_device *
sunneed_device_file_locker(const char *pathname);

char *
sunneed_device_get_dummy_file(const char *orig_path);

char *
get_path_from_dummy_path(const char *dummypath);

extern struct sunneed_device devices[];

int stepper_signal_fd, stepper_driver_pid;
int stepper_dataPipe[2];
int stepperMotor_orientation;
stepperMotor_Direction sunneed_stepperDir;
struct timespec *last_stepperMotor_req_time;
#endif
