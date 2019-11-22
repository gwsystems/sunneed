#include "bq27441.h"
#include <stdlib.h>

static int bq27441_file = -1;

static void assert_initialized(void) {
    if (bq27441_file < 0) {
        eprintf("you must run bq27441_init() first\n");
        exit(1);
    }
}

static uint16_t form_command(uint8_t command) {
    return (((uint16_t)command) << 8) | (command + 1);
}

static uint16_t form_control(uint8_t control) {
    return (uint16_t)control;
}

int bq27441_init(unsigned int bus_id) {
    int path_sz = 11;
    char i2c_path[path_sz];
    if (snprintf(i2c_path, path_sz, "/dev/i2c-%d", bus_id) > path_sz) {
        eprintf("unable to allocate path of I2C bus\n");
        return 1;
    }

    if ((bq27441_file = open(i2c_path, O_RDWR)) < 0) {
        eprintf("unable to acquire fd for device: %d\n", bq27441_file);
        return 1;
    }

    if (ioctl(bq27441_file, I2C_SLAVE, BQ27441_DEFAULT_I2C_ADDR) < 0) {
        eprintf("ioctl failure: %d\n", errno);
        return 1;
    }

    uint16_t device_id;
    if ((device_id = bq27441_device_id()) != BQ27441_DEVICE_ID) {
        eprintf("device ID is invalid; expected %d; got %d\n", BQ27441_DEVICE_ID, device_id);
        return 1;
    }

    return 0;
}

uint16_t bq27441_device_id(void) {
    assert_initialized();

    i2c_smbus_write_word_data(bq27441_file, 0x00, form_command(BQ27441_COMMAND_CONTROL));
    i2c_smbus_write_word_data(bq27441_file, 0x00, form_control(BQ27441_CONTROL_DEVICE_TYPE));

    return i2c_smbus_read_word_data(bq27441_file, 0x00);
}
