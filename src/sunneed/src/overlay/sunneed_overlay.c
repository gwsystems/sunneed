#include "sunneed_overlay.h"

void
on_load() {
#ifdef TESTING
    printf("PID %d\n", getpid());
#endif
}

void
on_unload() {
}

int
open(const char *pathname, int flags, mode_t mode) {
    printf("Opening file %s\n", pathname);

    int fd;
    SUPER(fd, open, int, (pathname, flags, mode), const char *, int, mode_t);

    printf("Got file handle %d\n", fd);

    return fd;
}
