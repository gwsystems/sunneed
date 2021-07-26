#include "../shared/sunneed_pip_interface.h"

struct sunneed_pip
pip_info() {
    return (struct sunneed_pip){"bq27441", 1000, 50};
}

unsigned int
present_power() {
    return 0;
    //return bq27441_nominal_avail_cap();
}
