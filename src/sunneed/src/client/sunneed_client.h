#include "../shared/sunneed_ipc.h"

typedef unsigned int sunneed_device_handle;

int
sunneed_get_device_handle(const char *identifier, sunneed_device_handle *handle);
