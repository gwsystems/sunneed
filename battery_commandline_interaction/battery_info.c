#include <stdlib.h>
#include <stdio.h>
#include <bq27441.h>
#include <dirent.h>
#include <string.h>

#define NUM_RECOGNIZED_ARGS 12

typedef struct arg_to_output{
    char *arg;
    int ret_type; /* 0 = uint, 1 = int */
    char *output_format;
    unsigned short (*func)(void);
    short (*func_int)(void);
} arg_to_output;

arg_to_output RECOGNIZED_ARGS[NUM_RECOGNIZED_ARGS] = {
    {
        .arg = "-cap",
	.ret_type = 0,
        .output_format = "Remaining capacity: %dmAh\n",
        .func = &bq27441_remaining_cap_unfiltered
    },
    {
	.arg = "-pwr",
	.ret_type = 1,
	.output_format = "Average Power: %dmW\n",
	.func_int = &bq27441_average_power
    },
    {
	.arg = "-temp",
	.ret_type = 0,
	.output_format = "Battery Temperature: %d x 10^-1 K\n",
	.func = &bq27441_temperature
    },
    {
	.arg = "-current",
	.ret_type = 0,
	.output_format = "Average current: %dmA\n",
	.func = &bq27441_average_current
    },
    {
	.arg = "-volt",
	.ret_type = 0,
	.output_format = "Voltage: %dmV\n",
	.func = &bq27441_voltage
    },
    {
	.arg = "-soc",
	.ret_type = 0,
	.output_format = "State of Charge: %d%%\n",
	.func = &bq27441_soc_unfiltered
    },
    {
	.arg = "-cap_full",
	.ret_type = 0,
	.output_format = "Battery Capacity @ full charge: %dmAh\n",
	.func = &bq27441_full_charge_cap
    },
    {
	.arg = "-cap_full_filt",
	.ret_type = 0,
	.output_format = "filtered capacity @ full charge: %dmAh\n",
	.func = &bq27441_full_charge_cap_filtered
    },
    {
	.arg = "-nom_cap",
	.ret_type = 0,
	.output_format = "Nominal Available Capacity (less than C/20load cap remaining): %d\n",
	.func = &bq27441_nominal_avail_cap
    },
    {
	.arg = "-cap_filtered",
	.ret_type = 0,
	.output_format = "Remaining cap filtered: %dmAh\n",
	.func = &bq27441_remaining_cap_filtered
    },
    {
	.arg = "-cap_unfiltered",
	.ret_type = 0,
	.output_format = "Remaining cap unfiltered: %dmAh\n",
	.func = &bq27441_remaining_cap_unfiltered
    },
    {
	.arg = "-soc_unfilt",
	.ret_type = 0,
	.output_format = "Uniltered state of charge: %d%%\n",
	.func = &bq27441_soc_unfiltered
    }
};

int
get_i2cbus_id(void)
{
    DIR *dev_dir;
    struct dirent *dev_entry;
    char prefix[5];

    dev_dir = opendir("/dev");
    if (dev_dir == NULL) {
        fprintf(stderr, "Unable to read /dev directory to get i2c bus id\n");
        abort();
    }
    while ( (dev_entry = readdir(dev_dir)) ) {
        strncpy(prefix, dev_entry->d_name, 4);
	if (!strncmp(prefix, "i2c-", strlen("i2c-"))) {
            return (int) dev_entry->d_name[4] - (int)'0';
        }
    }
    return -1;
}


int
main(int argc, char *argv[])
{
    int i, j, valid_arg;
    if (argc < 2) {
	fprintf(stderr, "No arguments given");
	exit(0);
    }
    if (bq27441_init(get_i2cbus_id())) {
        fprintf(stderr, "Battery Babysitter not connected\n");
        abort();
    }

    for (i = 1; i < argc; i++) {
	if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help") == 0) {
	    printf("Available arguments:\n\t-cap: Get remaining capacity of battery (mAh)\n\t-cap_filtered: Get filtered remaining capacity\n\t-cap_unfiltered: Get true battery capacity remaining\n\t-nom_cap: Get less than C/20 load battery capacity remaining\n\t-soc: Get state of charge (%%)\n\t-pwr: Get average power draw from battery\n\t-temp: Get temperature of battery\n\t-current: Get average current of battery\n\t-volt: Get voltage of battery\n");
	    continue;	
	}
	valid_arg = 0;
    	for (j = 0; j < NUM_RECOGNIZED_ARGS; j++) {
	    if (strcmp(argv[i], RECOGNIZED_ARGS[j].arg) == 0) {
    		switch (RECOGNIZED_ARGS[j].ret_type) {
		    case 0:
			printf(RECOGNIZED_ARGS[j].output_format, (RECOGNIZED_ARGS[j].func)());	
		    	break;
		    case 1:
			printf(RECOGNIZED_ARGS[j].output_format, (RECOGNIZED_ARGS[j].func_int)());
		        break;
		    default:
		    	fprintf(stderr, "Error calling func - invalid ret_type %d\n", RECOGNIZED_ARGS[j].ret_type);
			abort();
		}
		valid_arg = 1;
	    	break;
	    }
	}
	if (!valid_arg) {
	    fprintf(stderr, "Unknown argument: %s\n", argv[i]);
	    abort();
    	}	    
    }
     
}
