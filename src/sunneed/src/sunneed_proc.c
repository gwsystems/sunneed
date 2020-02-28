#include "sunneed_proc.h"

struct sunneed_tenant tenants[MAX_TENANTS];

int sunneed_init_tenants(void) {
    for (int i = 0; i < MAX_TENANTS; i++) {
        tenants[i] = (struct sunneed_tenant){
            .id = i,
            .pid = 0,
            .power_proportion = 0.0,
            .is_active = false
        };
    }

    return 0;
}

unsigned int sunneed_get_num_tenants(void) {
    unsigned int num_tenants = 0;

    for (int i = 0; i < MAX_TENANTS; i++) {
        if (tenants[i].is_active)
            num_tenants++;
    }

    return num_tenants;
}
