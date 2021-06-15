#include "../shared/sunneed_pip_interface.h"
#include "../../ext/libbq27441/bq27441.c"

struct sunneed_pip
pip_info() {
    return (struct sunneed_pip){"bq27441", 1000, 50};
}

unsigned int
present_power() {
    bq27441_init(1);
    return bq27441_nominal_avail_cap();
}
