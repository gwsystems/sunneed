#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#define NUM_REQUESTS 2000

int
main(void)
{
    int fd, orientation, i, dir, delay;
    char to_send[4];
    fd = open("/tmp/stepper", O_WRONLY);
    if (fd == -1) {
	fprintf(stderr, "Data Generator could not open file\n");
	return -1;
    }

    for (i = 0; i < NUM_REQUESTS; i++) {
	    orientation = rand() % 360;
        dir = rand() % 2;
        delay = rand() % 2;
        if (dir) {
            sprintf(to_send, "+%d", orientation);
            write(fd, to_send, strlen(to_send));
        } else {
            sprintf(to_send, "-%d", orientation);
            write(fd, to_send, strlen(to_send));
        }
        if (delay) usleep(300000);
        printf("%s\n",to_send);
    }
    
    return 0;
}
