#ifndef _SUNNEED_DEVICE_INTERFACE_H_
#define _SUNNEED_DEVICE_INTERFACE_H_

/* Runs once when module is loaded. */
int init(void);

/* Gets the data from the device. */
void *get(void *args);

double power_consumption(void *args);

#endif
