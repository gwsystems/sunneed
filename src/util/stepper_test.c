#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int
main(void)
{
	int fd = open("/tmp/stepper", O_WRONLY);
	if (fd == -1) {
		fprintf(stderr, "couldn't open file\n");
		return -1;
	}
	int dir = 200;
	printf("test: wrote %d bytes\n",write(fd, &dir, sizeof(dir)));

	dir = 300;
	write(fd, &dir, sizeof(dir));

	dir = 90;
	write(fd, &dir, sizeof(dir));
	return 0;
}

