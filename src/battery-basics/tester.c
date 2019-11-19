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
uint16_t read_word(char word, int file){
	uint8_t recv_data[2];
	recv_data[0] = i2c_smbus_read_byte_data(file, word);
	recv_data[1] = i2c_smbus_read_byte_data(file, word+1);
	uint16_t data = recv_data[0]| recv_data[1]<<8;
    return data;
}

//Checks to make sure device type is right
int device_type(int file){
	//TODO test that this works without the last 0x00 that we had before
    char buf[] = {0x00, 0x01, 0x00};
	write(file,buf,4);	
	uint8_t recv_data[2];
	recv_data[0] = i2c_smbus_read_byte_data(file, 0x00);
	recv_data[1] = i2c_smbus_read_byte_data(file, 0x01);
	uint16_t data = recv_data[0]| recv_data[1]<<8;
	printf("Device type is %x\n", data);
    if (data!=0x421){
        return -1;
    }
    else{
        return 0;
    }
}
//Voltage in mV
uint16_t read_voltage(int file){
    char code = 0x04;
    return read_word(code,file);
}
//avg current in mA
//current is a signed int cuz it can be negative or positive
int16_t read_current(int file){
    char code= 0x10;
    return (int16_t)read_word(code,file);
}
//remaining capacity in mAh
uint16_t read_capacity(int file){
    char code = 0x0C;
    return read_word(code,file);
}
//state of charge unfiltered in percent
uint16_t read_soc(int file){
    char code=0x1C;
    return read_word(code,file);
}
//temp of the battery in 0.1K 
uint16_t read_temp(int file){
    char code=0x02;
    return read_word(code,file);
}

void print_test(void){
	
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
    if (device_type(file)!=0){
        printf("Device type doesn't match! i2c error");
        exit(1);
    }
    uint16_t voltage = read_voltage(file);
    int16_t current = read_current(file);
    uint16_t capacity = read_capacity(file);
    uint16_t state_of_charge = read_soc(file);
    uint16_t temp = read_temp(file);
    printf("Voltage is %i mV\n",voltage);
    printf("Current is %i mA\n",current);
    printf("Capacity is %i mAh\n",capacity);
    printf("SoC is at %i percent\n",state_of_charge);
    printf("Temp is %i 0.1K\n",temp);
}

void main(void) {
	print_test();
}
