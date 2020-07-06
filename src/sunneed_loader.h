#include <dirent.h>
#include <dlfcn.h>
#include <stdlib.h>

#include "sunneed_device.h"

#define DEVICE_PATH_LEN 64

#define OBJ_EXTENSION ".so"
#define OBJ_EXTENSION_LEN 3

int
sunneed_load_devices(struct sunneed_device *target);
