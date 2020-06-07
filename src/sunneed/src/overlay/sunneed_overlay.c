#include "sunneed_overlay.h"

void
on_load() {
#ifdef TESTING
    printf("PID %d\n", getpid());
#endif
}
