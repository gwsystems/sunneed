#include "../shared/sunneed_device_interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define UNUSED(X) (void)(X)

int
init(void) {
    srand(time(NULL));
    return 0;
}

void *
get(void *args) {
    int val = rand() % 256;
    *(int *)args = val; // sorry...
    return args;
}

double
power_consumption(void *args) {
    UNUSED(args);
    return *(double *)args;
}
