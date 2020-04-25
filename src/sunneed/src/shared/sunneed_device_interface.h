#ifndef _SUNNEED_DEVICE_INTERFACE_H_
#define _SUNNEED_DEVICE_INTERFACE_H_

/* Runs once when module is loaded. */
extern int
init(void);

/* Gets the data from the device. */
extern void *
get(void *args);

extern double
power_consumption(void *args);

#endif
