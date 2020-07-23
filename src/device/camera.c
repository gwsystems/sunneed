#include "../shared/sunneed_device_interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNUSED(X) (void)(X)

enum sunneed_device_type device_type_kind = DEVICE_TYPE_FILE_LOCK;
struct sunneed_device_type_file_lock device_type_data = {
    .files = CAMERA_PATH
};

int
init(void) {
    return 0;
}

void *
get(void *args) {
    UNUSED(args);
    printf("Hello from 'camera'!\n");
    return args;
}

double
power_consumption(void *args) {
    UNUSED(args);
    return *(double *)args;
}

void *
get_device_type_data(void) {
    return &device_type_data;
}
