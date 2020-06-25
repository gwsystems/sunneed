#include <dirent.h>
#include <dlfcn.h>
#include <stdlib.h>

#include "sunneed_device.h"

#define DEVICE_PATH_LEN 64

#define OBJ_EXTENSION ".so"
#define OBJ_EXTENSION_LEN 3

struct sunneed_device *
sunneed_load_devices(void);
