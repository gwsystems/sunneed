#include "../shared/sunneed_device_interface.h"
#include <stdlib.h>

#define UNUSED(X) (void)(X)

#define MAX_USAGE 20

int
init(void) {
    return 0;
}

void *
get(void *args) {
    int *buf = malloc(sizeof(int));
    *buf = *(int *)args;
    return (void *)buf;
}

double
power_consumption(void *args) {
    UNUSED(args);
    return *(double *)args;
}
