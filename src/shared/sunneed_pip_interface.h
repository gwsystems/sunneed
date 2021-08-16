#ifndef _SUNNEED_PIP_H_
#define _SUNNEED_PIP_H_
#include "../../ext/libbq27441/bq27441.h"

/*
 * Describes the interface for a power information provider (a PIP).
 * A PIP is what tells sunneed statistics about the battery, such as power and
 *  electrical current usage.
 * This API is unstable and new things will definitely be added as we progress
 *  in sunneed's development.
 */

struct sunneed_pip {
    // A textual name to identify the PIP by. Probably won't be used in any
    //  actual logic.
    const char *name;

    // The maximum power the system can have. Units are arbitrary, as this
    //  value will only be used in comparison to the present power.
    unsigned int max_power;

    // The interval, in milliseconds, that the device can update its power
    //  statistics. This helps reduce unnecessary device accesses by
    //  sunneed.
    // TODO: Some devices may have different updatee intervals across their
    //  components; should we account for that?
    unsigned int update_interval;
};

struct sunneed_pip
pip_info();

signed int
present_power();

#endif
