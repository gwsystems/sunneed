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

extern enum sunneed_device_type
get_device_type_kind(void);

extern const void *
get_device_type(void);

#endif
