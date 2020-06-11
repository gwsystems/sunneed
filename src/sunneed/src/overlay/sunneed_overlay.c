#include "sunneed_overlay.h"

void
on_load() {
#ifdef TESTING
    printf("PID %d\n", getpid());
#endif
    
    sunneed_client_init("TODO");

    sunneed_device_handle_t handle;

    // TODO We are manually registering camera because a device needs to be loaded in order for its lockfiles to be
    //  read. We need some kind of solution to automatically load when one of their lockfiles is accessed.
    sunneed_client_get_device_handle("camera", &handle);
}

void
on_unload() {
}

int
open(const char *pathname, int flags, mode_t mode) {
    printf("Opening file %s\n", pathname);

    // This should pause and wait for resources for the camera; the return value isn't that important.
    int locker = sunneed_client_check_locked_file(pathname);

    int fd;
    SUPER(fd, open, int, (pathname, flags, mode), const char *, int, mode_t);

    printf("Got file handle %d\n", fd);

    return fd;
}
