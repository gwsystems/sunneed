#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int
main(void)
{
	int fd = open("/tmp/stepper", O_WRONLY);
	if (fd == -1) {
		fprintf(stderr, "couldn't open file\n");
		return -1;
	}
	int dir = 200;
    write(fd, &dir, sizeof(dir));

	dir = 300;
	write(fd, &dir, sizeof(dir));

	dir = 90;
	write(fd, &dir, sizeof(dir));

	char to_send[10];
	sprintf(to_send, "+%d", 90);
	write(fd, to_send, strlen(to_send));
	sprintf(to_send, "-%d", 355);
	write(fd, to_send, strlen(to_send));
	return 0;
}

