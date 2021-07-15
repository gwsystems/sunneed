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
        char *dummy_path = sunneed_client_fetch_locked_file_path(pathname, flags, mode);
        pathname = dummy_path;
    }

    int fd;
    SUPER(fd, open, int, (pathname, flags, mode), const char *, int, mode_t);

    // TODO Handle errors from open

    sunneed_client_on_locked_path_open(locked, (char *)pathname, fd);

    sunneed_client_debug_print_locked_path_table();

    return fd;
}

ssize_t
write(int fd, const void *buf, size_t count) {
    printf("Overlay write %d\n", fd);
    int ret;

    if (!sunneed_client_fd_is_locked(fd)) {
        // Perform the write as normal.
        SUPER(ret, write, int, (fd, buf, count), int, const void *, size_t);
        return ret;
    }
    
    // Ask sunneed to do the write for us.
    sunneed_client_remote_write(fd, buf, count);

    return 0;
}
