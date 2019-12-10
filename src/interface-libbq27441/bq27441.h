#ifndef _BQ27441_H
#define _BQ27441_H

#include <i2c/smbus.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
 * Commands are two bytes. The byte defined here is the first byte; the second
 * one will always be the (FirstByte + 0x01). That byte MUST be written after one
 * of the commands specified here.
 *
 * Furthermore, please note that many of these are not implemented. This
 * library is being written on an as-needed basis for other projects of the
 * author's.
 */
#define BQ27441_COMMAND_CONTROL                    0x00
#define BQ27441_COMMAND_TEMPERATURE                0x02
#define BQ27441_COMMAND_VOLTAGE                    0x04
#define BQ27441_COMMAND_FLAGS                      0x06
#define BQ27441_COMMAND_NOMINAL_AVAIL_CAP          0x08
#define BQ27441_COMMAND_FULL_AVAIL_CAP             0x0A
#define BQ27441_COMMAND_REMAINING_CAP              0x0C
#define BQ27441_COMMAND_FULL_CHARGE_CAP            0x0E
#define BQ27441_COMMAND_AVERAGE_CURRENT            0x10
#define BQ27441_COMMAND_STANDBY_CURRENT            0x12
#define BQ27441_COMMAND_MAX_LOAD_CURRENT           0x14
#define BQ27441_COMMAND_AVERAGE_POWER              0x18
#define BQ27441_COMMAND_STATE_OF_CHARGE            0x1C
#define BQ27441_COMMAND_INTERNAL_TEMPERATURE       0x1E
#define BQ27441_COMMAND_STATE_OF_HEALTH            0x20
#define BQ27441_COMMAND_REMAINING_CAP_UNFILTERED   0x28
#define BQ27441_COMMAND_REMAINING_CAP_FILTERED     0x2A
#define BQ27441_COMMAND_FULL_CHARGE_CAP_UNFILTERED 0x2C
#define BQ27441_COMMAND_FULL_CHARGE_CAP_FILTERED   0x2E
#define BQ27441_COMMAND_SOC_UNFILTERED             0x30

/*
 * Control commands are two bytes, but the first is alwas 0x00. Therefore when
 * writing using these constants you MUST write 0x00 before the actual command
 * value.
 *
 * Furthermore, please note that many of these are not implemented. This
 * library is being written on an as-needed basis for other projects of the
 * author's.
 */
#define BQ27441_CONTROL_STATUS          0x00
#define BQ27441_CONTROL_DEVICE_TYPE     0x01
#define BQ27441_CONTROL_FW_VERSION      0x02
#define BQ27441_CONTROL_DM_CODE         0x04
#define BQ27441_CONTROL_PREV_MACWRITE   0x07
#define BQ27441_CONTROL_CHEM_ID         0x08
#define BQ27441_CONTROL_BAT_INSERT      0x0C
#define BQ27441_CONTROL_BAT_REMOVE      0x0D
#define BQ27441_CONTROL_SET_HIBERNATE   0x11
#define BQ27441_CONTROL_CLEAR_HIBERNATE 0x12
#define BQ27441_CONTROL_SET_CFGUPDATE   0x13
#define BQ27441_CONTROL_SHUTDOWN_ENABLE 0x1B
#define BQ27441_CONTROL_SHUTDOWN        0x1C
#define BQ27441_CONTROL_SEALED          0x20
#define BQ27441_CONTROL_TOGGLE_GPUOUT   0x23
#define BQ27441_CONTROL_RESET           0x41
#define BQ27441_CONTROL_SOFT_RESET      0x42
#define BQ27441_CONTROL_EXIT_CFGUPDATE  0x43
#define BQ27441_CONTROL_EXIT_RESIM      0x44

#define BQ27441_DEVICE_ID 0x0421

#define BQ27441_DEFAULT_I2C_ADDR 0x55

#define eprintf(format, ...) \
    fprintf(stderr, "bq27441 error: " format, ##__VA_ARGS__)

typedef struct {
    unsigned int shutdown_enabled : 1;
    unsigned int watchdog_reset : 1;
    unsigned int sealed_state : 1;
    unsigned int calibration_mode : 1;
    unsigned int cc_calibration_active : 1;
    unsigned int board_calibration_active : 1;
    unsigned int qmax_updated : 1;
    unsigned int resistance_updated : 1;

    unsigned int init_complete : 1;
    unsigned int hibernate_request : 1;
    unsigned int _reserved_0 : 1;
    unsigned int sleep_mode : 1;
    unsigned int constant_power_model : 1;
    unsigned int ra_table_updates_disabled : 1;
    unsigned int voltage_updates_okay : 1;
    unsigned int _reserved_1 : 1;
} bq27441_status_t;

int bq27441_init(unsigned int bus_id);

/* Control subcommands. */
bq27441_status_t bq27441_status(void);
uint16_t bq27441_device_id(void);
uint16_t bq27441_firmware_version(void);
uint16_t bq27441_dm_code(void);
uint16_t bq27441_prev_macwrite(void);
uint16_t bq27441_chem_id(void);
uint16_t bq27441_temperature(void);
uint16_t bq27441_voltage(void);
uint16_t bq27441_flags(void);
uint16_t bq27441_nominal_avail_cap(void);
uint16_t bq27441_full_avail_cap(void);
uint16_t bq27441_remaining_cap(void);
uint16_t bq27441_full_charge_cap(void);
uint16_t bq27441_average_current(void);
uint16_t bq27441_standby_current(void);
uint16_t bq27441_max_load_current(void);
uint16_t bq27441_average_power(void);
uint16_t bq27441_state_of_charge(void);
uint16_t bq27441_internal_temperature(void);
uint16_t bq27441_state_of_health(void);
uint16_t bq27441_remaining_cap_unfiltered(void);
uint16_t bq27441_remaining_cap_filtered(void);
uint16_t bq27441_full_charge_cap_unfiltered(void);
uint16_t bq27441_full_charge_cap_filtered(void);
uint16_t bq27441_soc_unfiltered(void);

#endif
