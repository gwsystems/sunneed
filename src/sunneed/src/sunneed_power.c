#include "sunneed_power.h"

extern struct sunneed_tenant tenants[];

static struct {
    // Index.
    int id;

    // Power at the start of the quantum.
    int present_power;

    // Timestamp for the start of the quantum.
    struct timespec begin_time;

    // Whether events can be recorded to this quantum.
    bool is_active;
} current_quantum = { -1, 0.0, { 0 }, false };

int sunneed_record_power_usage_event(struct sunneed_power_usage_event ev) {
    if (!current_quantum.is_active) {
        LOG_E("Cannot record a power event outside of an active quantum");
        return 1;
    }

    struct sunneed_power_usage_event *cur = power_usage_evs;
    if (cur == NULL) {
        LOG_E("Power usage events head node is unallocated!");
        return 1;
    }

    // Find tail of events list.
    while (cur->next != NULL) {
        cur = cur->next;
    }

    // Copy given event to heap and attach to end of events list.
    cur->next = malloc(sizeof(struct sunneed_power_usage_event));
    if (!cur->next) {
        LOG_E("Failed to allocate space for power usage event.");
        return 1;
    }
    *cur->next = ev;

    return 0;
}

int sunneed_quantum_begin(void) {
    LOG_D("Begin start procedure for quantum %d", current_quantum.id + 1);

    // Set quantum metadata.
    current_quantum.id++;
    current_quantum.present_power = present_power();
    timespec_get(&current_quantum.begin_time, TIME_UTC);

    // Reset power events array.
    struct sunneed_power_usage_event *cur = power_usage_evs;
    while (cur != NULL) {
        struct sunneed_power_usage_event *next = cur->next;
        free(cur);
        cur = next;
    }

    power_usage_evs = malloc(sizeof(struct sunneed_power_usage_event));
    if (!power_usage_evs) {
        LOG_E("Failed to allocate space for power usage events!");
        return 1;
    }

    LOG_I("Started quantum %d", current_quantum.id);
    current_quantum.is_active = true;

    return 0;
}

int sunneed_quantum_end(void) {
    LOG_I("Ending quantum %d", current_quantum.id);
    
    current_quantum.is_active = false;

    double power_consumed[MAX_TENANTS] = {0.0};

    // Add up power used in this quantum by each tenant.
    struct sunneed_power_usage_event *ev = power_usage_evs;
    while (ev != NULL) {
        power_consumed[ev->ev.tenant->id] += ev->ev.device->power_consumption;
        ev = ev->next;
    }

    float unscaled_proportions[MAX_TENANTS] = {0.0};
    float unscaled_sum = 0.0;

    // Update power proportions for tenants.
    
    // First, get the percentage of their given power that each tenant used.
    for (int i = 0; i < MAX_TENANTS; i++) {
        unscaled_proportions[i] = 
            1.0 - power_consumed[i] / (current_quantum.present_power * tenants[i].power_proportion);
        unscaled_sum += unscaled_proportions[i];
    }

    // Multiply the total percentage by a scaling factor such that the sum of new proportions adds up to 1.
    float scale_factor = 1.0 / unscaled_sum;
    for (int i = 0; i < MAX_TENANTS; i++) {
        tenants[i].power_proportion = unscaled_proportions[i] * scale_factor;
    }

    LOG_D("Finished ending quantum %d", current_quantum.id);

    return 0;
}
