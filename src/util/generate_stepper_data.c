#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define NUM_REQUESTS 20

int
main(void)
{
    int fd, orientation, i;
    fd = open("/tmp/stepper", O_WRONLY);
    if (fd == -1) {
	fprintf(stderr, "Data Generator could not open file\n");
	return -1;
    }

    for (i = 0; i < NUM_REQUESTS; i++) {
	orientation = rand() % 360;
	write(fd, &orientation, sizeof(orientation));
	usleep(5000);	
    }

    return 0;
}
