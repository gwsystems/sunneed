/* An example of a broken `sunneed` device -- it does not define `power_consumption`. */

#include "../shared/sunneed_device_interface.h"
#include "../shared/sunneed_testing.h"
#include <stdio.h>
#include <stdlib.h>

#define UNUSED(X) (void)(X)

int
init(void) {
    return 0;
}

void *
get(void *args) {
    UNUSED(args);
    return TEST_DEVICE_OUTPUT;
}

unsigned int
device_flags(void) {
    return SUNNEED_DEVICE_FLAG_SILENT_FAIL;
}
