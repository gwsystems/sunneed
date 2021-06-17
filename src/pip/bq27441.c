#include "../shared/sunneed_pip_interface.h"
#include "../../ext/libbq27441/bq27441.c"
#include "../log.h"
#include <dirent.h>
#include <string.h>

int
pip_init(void) {
    DIR *dev_dir;
    struct dirent *entry;
    char entry_prefix[5];
    int i2c_pathNum = -1;

    entry_prefix[4] = '\0';

    if ( (dev_dir = opendir("/dev")) == NULL) {
	LOG_E("Unable to get path for i2c");
	return 1;
    }
        
    while ( (entry = readdir(dev_dir)) != NULL) {
	strncpy(entry_prefix, entry->d_name, 4);
	if (strcmp(entry_prefix, "i2c-") == 0) {
	    i2c_pathNum = (int)entry->d_name[4] - (int)'0';
	    break;
	}
    }

    if (i2c_pathNum != -1) return bq27441_init(i2c_pathNum);

    return 1;
}


struct sunneed_pip
pip_info() {
    return (struct sunneed_pip){"bq27441", 1000, 50};
}

unsigned int
present_power() {
    return bq27441_nominal_avail_cap();
}
