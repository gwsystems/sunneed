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

    int locked = sunneed_client_check_locked_file(pathname);
    if (locked < 0) {
        printf("'%s' is not locked; opening normally\n", pathname);
    } else {
        printf("'%s' is locked; opening via dummy\n", pathname);
        char *dummy_path = sunneed_client_open_locked_file(pathname);
        pathname = dummy_path;
    }

    int fd;
    SUPER(fd, open, int, (pathname, flags, mode), const char *, int, mode_t);

    printf("Got file handle %d\n", fd);

    return fd;
}
