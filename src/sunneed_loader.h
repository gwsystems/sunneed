#ifndef _SUNNEED_LOADER_H_
#define _SUNNEED_LOADER_H_

#include <dirent.h>
#include <dlfcn.h>
#include <stdlib.h>

#include "sunneed_device.h"

#define DEVICE_PATH_LEN 64

#define OBJ_EXTENSION ".so"
#define OBJ_EXTENSION_LEN 3

int
sunneed_load_devices(struct sunneed_device *target);

#ifdef TESTING

#include "shared/sunneed_testing.h"
#include "sunneed_test.h"

int TEST_load_device(void);
int TEST_load_broken_device(void);

#define SUNNEED_RUNTIME_TESTS_LOADER \
    TEST_load_device, \
    TEST_load_broken_device

#endif

#endif
