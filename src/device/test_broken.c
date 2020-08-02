// An example of a broken device -- it does not define a `get_device_type_data`.

#include "../shared/sunneed_device_interface.h"
#include "../shared/sunneed_testing.h"
#include <stdio.h>
#include <stdlib.h>

int
init(void) {
    return 0;
}

enum sunneed_device_type device_type_kind = DEVICE_TYPE_FILE_LOCK;

unsigned int device_flags = SUNNEED_DEVICE_FLAG_SILENT_FAIL;
