#include "sunneed_overlay.h"

void
on_load() {
    sunneed_client_init("TODO");

    printf("Overlay: Client init\n");
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
        char *dummy_path = sunneed_client_fetch_locked_file_path(pathname);
        pathname = dummy_path;
    }

    int fd;
    SUPER(fd, open, int, (pathname, flags, mode), const char *, int, mode_t);

    // TODO Handle errors from open

    sunneed_client_on_locked_path_open(locked, (char *)pathname, fd);

    sunneed_client_debug_print_locked_path_table();

    return fd;
}
