#ifndef _SUNNEED_DEVICE_INTERFACE_H_
#define _SUNNEED_DEVICE_INTERFACE_H_

#include "sunneed_device_type.h"
#include "../protobuf/c/device.pb-c.h"

/** 
 * Runs once when module is loaded.
 * If the return value is greater than zero, loading will be aborted with an error.
 * If the return value is less than zero, loading will be silently aborted.
 */
extern int
init(void);

/**
 * The type of the implementing device.
 * Under the hood, defines the union member of `device_type_data` to write to.
 */
extern enum sunneed_device_type device_type_kind;

/** Should return a pointer to a struct, the type of which corresponds with the `device_type_kind` of this.  */
extern void *
get_device_type_data(void);

/** A set of bitflags as defined in `sunneed_device_interface.h`. */
extern unsigned int device_flags;

#endif
