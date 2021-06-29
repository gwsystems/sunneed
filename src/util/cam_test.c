#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

int
main(void)
{
    int dev_fd;
    const char *path = "/tmp/cam_driver";

    if ( (dev_fd = open(path, O_WRONLY)) == -1) {
        fprintf(stderr, "ERR: could not open device\n");
        exit(1);
    }

    write(dev_fd, "hello :)\n", strlen("hello :)\n"));

    close(dev_fd);

    return 0;
}