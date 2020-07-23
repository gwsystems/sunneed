#ifndef _SUNNEED_DEVICE_INTERFACE_H_
#define _SUNNEED_DEVICE_INTERFACE_H_

#include "sunneed_device_type.h"
#include "../protobuf/c/device.pb-c.h"

/* Runs once when module is loaded. */
extern int
init(void);

/* Gets the data from the device. */
extern void *
get(void *args);

extern double
power_consumption(void *args);

extern enum sunneed_device_type device_type_kind;

extern void *
get_device_type_data(void);

#endif
