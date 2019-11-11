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
	char buf[10];
	buf[0] = 0x00;
	buf[1] = 0x01;
	buf[2] = 0x00;
	buf[3] = 0x00;

	uint8_t recv_data[2];
	write(file,buf,4);	
	recv_data[0] = i2c_smbus_read_byte_data(file, 0x00);
	recv_data[1] = i2c_smbus_read_byte_data(file, 0x01);
	uint16_t data = recv_data[0]| recv_data[1]<<8;

	printf("Device type: 0x%x 0x%x\n", recv_data[0], recv_data[1]);
	printf("data is 0x%x\n",data);
}
