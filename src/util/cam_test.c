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
    const char *path = "/tmp/camera";
    if ( (dev_fd = open(path, O_WRONLY)) == -1) {
        fprintf(stderr, "ERR: could not open device fd\n");
        exit(1);
    }

    write(dev_fd, "start_preview()\n", strlen("start_preview()\n"));
    write(dev_fd, "capture('img.jpg')\n", strlen("capture('img.jpg')\n"));
    write(dev_fd, "stop_preview()\n", strlen("stop_preview()\n"));
    close(dev_fd);

    return 0;
}