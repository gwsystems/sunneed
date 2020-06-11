// Example of a program that would run on a sunneed system. This program should be run with `LD_PRELOAD` overlaying
//  our custom functions. This program doesn't reference sunneed in any way; the design is that communication
//  should be automatic.

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// Just to get the CAMERA_PATH.
#include "../shared/sunneed_device_type.h"

int
main(void) {
    printf("PID %d\n", getpid());

    int fd = open(CAMERA_PATH, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        fprintf(stderr, "Failed to open file\n");
        return -1;
    }

    close(fd);

    return 0;
}
