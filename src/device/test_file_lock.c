// An example device that implements the bare minimum.

#include "../shared/sunneed_device_interface.h"
#include "../shared/sunneed_testing.h"
#include <stdio.h>
#include <stdlib.h>

struct sunneed_device_type_file_lock data = {
    .len = 1,
    .paths = { TEST_FILE_LOCK_FILE_PATH }
};

int
init(void) {
    return 0;
}

enum sunneed_device_type device_type_kind = DEVICE_TYPE_FILE_LOCK;

void *
get_device_type_data(void) {
    return &data;
}

unsigned int device_flags = 0;

// TODO Device type.
