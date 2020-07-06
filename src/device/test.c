/* The tester to rule them all. This is just a device that returns a string. */

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

double
power_consumption(void *args) {
    UNUSED(args);
    return 0;
}

// TODO Device type.
