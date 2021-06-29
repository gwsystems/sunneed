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
#include "../shared/sunneed_testing.h"

int
main(void) {
    printf("Starting main overlay tester\n");
    printf("tenant: flags - %d\n",O_CREAT | O_RDWR);
    int fd = open("/tmp/test", O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        fprintf(stderr, "Failed to open file\n");
        return -1;
    }

    write(fd, "foo", 3);

    close(fd);

    printf("Opened and closed file\n");

    return 0;
}
