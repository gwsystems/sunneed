#include <i2c/smbus.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>

void main(void) {
	const char *filename = "/dev/i2c-1";
	int file = open(filename, O_RDWR);
	if (file < 0) {
		// TODO Error handling.
		exit(1);
	}

	if (ioctl(file, I2C_SLAVE, 0x55) < 0) {
		printf("ioctl error: %s\n", strerror(errno));
		exit(1);
	}

	uint8_t writes[2] = { 0x00, 0x01 };

	write(file, &writes[0], 1);
	write(file, &writes[1], 1);
	write(file, &writes[0], 1);
	write(file, &writes[1], 1);
	
	uint8_t recv_data[2];
	recv_data[0] = i2c_smbus_read_byte_data(file, 0x00);
	recv_data[1] = i2c_smbus_read_byte_data(file, 0x01);

	printf("Device type: 0x%x 0x%x\n", recv_data[0], recv_data[1]);
}
