#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int
main(void)
{
	int fd = open("/dev/stepper", O_WRONLY);
	if (fd == -1) {
		fprintf(stderr, "couldn't open file\n");
		return -1;
	}
	int dir = 90;
	write(fd, &dir, sizeof(dir));

	close(fd);

	return 0;
}

