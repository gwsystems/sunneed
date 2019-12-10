#include "bq27441.h"
#include <stdlib.h>

union bq27441_status_int {
    bq27441_status_t status;
    uint16_t integer;
};

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

static void control_command(uint8_t control) {
    assert_initialized();

    i2c_smbus_write_word_data(bq27441_file, 0x00, form_command(BQ27441_COMMAND_CONTROL));
    i2c_smbus_write_word_data(bq27441_file, 0x00, (uint16_t)control);
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

bq27441_status_t bq27441_status(void) {
    control_command(BQ27441_CONTROL_STATUS);
    // TODO Make sure this works.
    return ((union bq27441_status_int)(uint16_t)i2c_smbus_read_word_data(bq27441_file, 0x00)).status;
}

uint16_t bq27441_device_id(void) {
    control_command(BQ27441_CONTROL_DEVICE_TYPE);
    return i2c_smbus_read_word_data(bq27441_file, 0x00);
}

uint16_t bq27441_firmware_version(void) {
    control_command(BQ27441_CONTROL_FW_VERSION);
    return i2c_smbus_read_word_data(bq27441_file, 0x00);
}

uint16_t bq27441_dm_code(void) {
    control_command(BQ27441_CONTROL_DM_CODE);
    return i2c_smbus_read_word_data(bq27441_file, 0x00);
}

uint16_t bq27441_prev_macwrite(void) {
    control_command(BQ27441_CONTROL_PREV_MACWRITE);
    return i2c_smbus_read_word_data(bq27441_file, 0x00);
}

uint16_t bq27441_chem_id(void) {
    control_command(BQ27441_CONTROL_CHEM_ID);
    return i2c_smbus_read_word_data(bq27441_file, 0x00);
}

uint16_t bq27441_temperature(void) {
    return i2c_smbus_read_word_data(bq27441_file, BQ27441_COMMAND_TEMPERATURE);
}

// TODO Flags()

uint16_t bq27441_voltage(void) {
    return i2c_smbus_read_word_data(bq27441_file, BQ27441_COMMAND_VOLTAGE);
}

uint16_t bq27441_nominal_avail_cap(void) {
    return i2c_smbus_read_word_data(bq27441_file, BQ27441_COMMAND_NOMINAL_AVAIL_CAP);
}

uint16_t bq27441_full_avail_cap(void) {
    return i2c_smbus_read_word_data(bq27441_file, BQ27441_COMMAND_FULL_AVAIL_CAP);
}

uint16_t bq27441_remaining_cap(void) {
    return i2c_smbus_read_word_data(bq27441_file, BQ27441_COMMAND_REMAINING_CAP);
}

uint16_t bq27441_full_charge_cap(void) {
    return i2c_smbus_read_word_data(bq27441_file, BQ27441_COMMAND_FULL_CHARGE_CAP);
}

uint16_t bq27441_average_current(void) {
    return i2c_smbus_read_word_data(bq27441_file, BQ27441_COMMAND_AVERAGE_CURRENT);
}

uint16_t bq27441_standby_current(void) {
    return i2c_smbus_read_word_data(bq27441_file, BQ27441_COMMAND_STANDBY_CURRENT);
}

uint16_t bq27441_max_load_current(void) {
    return i2c_smbus_read_word_data(bq27441_file, BQ27441_COMMAND_MAX_LOAD_CURRENT);
}

uint16_t bq27441_average_power(void) {
    return i2c_smbus_read_word_data(bq27441_file, BQ27441_COMMAND_AVERAGE_POWER);
}

uint16_t bq27441_state_of_charge(void) {
    return i2c_smbus_read_word_data(bq27441_file, BQ27441_COMMAND_STATE_OF_CHARGE);
}

uint16_t bq27441_internal_temperature(void) {
    return i2c_smbus_read_word_data(bq27441_file, BQ27441_COMMAND_INTERNAL_TEMPERATURE);
}

// TODO StateOfHealth()

uint16_t bq27441_remaning_cap_unfiltered(void) {
    return i2c_smbus_read_word_data(bq27441_file, BQ27441_COMMAND_REMAINING_CAP_UNFILTERED);
}

uint16_t bq27441_remaning_cap_filtered(void) {
    return i2c_smbus_read_word_data(bq27441_file, BQ27441_COMMAND_REMAINING_CAP_FILTERED);
}

uint16_t bq27441_full_charge_cap_unfiltered(void) {
    return i2c_smbus_read_word_data(bq27441_file, BQ27441_COMMAND_FULL_CHARGE_CAP_UNFILTERED);
}

uint16_t bq27441_full_charge_cap_filtered(void) {
    return i2c_smbus_read_word_data(bq27441_file, BQ27441_COMMAND_FULL_CHARGE_CAP_FILTERED);
}

uint16_t bq27441_soc_unfiltered(void) {
    return i2c_smbus_read_word_data(bq27441_file, BQ27441_COMMAND_SOC_UNFILTERED);
}
