#include "sunneed_overlay.h"

void
on_load() {
    sunneed_client_init("TODO");
}

void
on_unload() {
    sunneed_client_disconnect();
}

int
open(const char *pathname, int flags, mode_t mode) {
    printf("Opening file %s\n", pathname);

    // This should pause and wait for resources for the camera; the return value isn't that important.
    sunneed_client_check_locked_file(pathname);

    int fd;
    SUPER(fd, open, int, (pathname, flags, mode), const char *, int, mode_t);

    printf("Got file handle %d\n", fd);

    return fd;
}
