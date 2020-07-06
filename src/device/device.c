#include "../shared/sunneed_device_interface.h"
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
    printf("Hello from 'device'!\n");
    return args;
}

double
power_consumption(void *args) {
    UNUSED(args);
    return *(double *)args;
}

// TODO Device type.
