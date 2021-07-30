// Example of a program that would run on a sunneed system. This program should be run with `LD_PRELOAD` overlaying
//  our custom functions. This program doesn't reference sunneed in any way; the design is that communication
//  should be automatic.

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

// Just to get the CAMERA_PATH.
#include "../shared/sunneed_device_type.h"
#include "../shared/sunneed_testing.h"

int
main(void) {
    printf("Starting main overlay tester\n");
//    int fd = open("/tmp/test", O_CREAT | O_RDWR, 0666);
    int fd = open("/tmp/test", O_RDWR);
    if (fd == -1) {
        exit(1);
    }
    printf("%d",fd);
    write(fd, "foo", 3);
 //   close(fd);

    fd = open("/tmp/test", O_RDONLY);
    write(fd, "foo", 3);
    printf("%d",fd);
    printf("Opened and closed file\n");

    return 0;
}
