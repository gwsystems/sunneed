#include "sunneed_device.h"

struct sunneed_device devices[MAX_DEVICES];

void *sunneed_device_get_device_type(struct sunneed_device *device) {
    switch (device->device_type_kind) {
        case DEVICE_TYPE_FILE_LOCK:
            return &device->device_type.file_lock;
    }

    return NULL;
}

bool
sunneed_device_is_linked(struct sunneed_device *device) {
    if (device->get == NULL || device->power_consumption == NULL)
        return false;
    return true;
}

/** 
 * Check for devices locking the specified pathname.
 * Returns the device specifying the lock if one is found, otherwise returns NULL.
 */
struct sunneed_device *
sunneed_device_file_is_locked(const char *pathname) {
    for (int i = 0; i < MAX_DEVICES; i++) {
        // Find devices of type FILE_LOCK.
        if (!devices[i].is_linked || devices[i].device_type_kind != DEVICE_TYPE_FILE_LOCK)
            continue;

        LOG_D("Comparing %s to %s", pathname, devices[i].device_type.file_lock.files);
        
        // TODO Treat locked filepaths as a list of paths.
        if (!strncmp(pathname, devices[i].device_type.file_lock.files, strlen(pathname))) {
            printf("Locked file\n");
            return &devices[i];
        }
    }

    return NULL;
}
