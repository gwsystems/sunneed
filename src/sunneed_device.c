#include "sunneed_device.h"

struct sunneed_device devices[MAX_DEVICES];
// TODO FD's make no sense here!
struct {
    int fd;
    char pathname[64]; // TODO Don't hardcode.
    char dummypath[64];
} dummy_fd_map[MAX_LOCKED_FILES] = { { -1, { '\0' }, { '\0' } } };

/** 
 * Check for devices locking the specified pathname.
 * Returns the device specifying the lock if one is found, otherwise returns NULL.
 */
struct sunneed_device *
sunneed_device_file_locker(const char *pathname) {
    for (int i = 0; i < MAX_DEVICES; i++) {
        // Find devices of type FILE_LOCK.
        if (!devices[i].is_ready || devices[i].device_type_kind != DEVICE_TYPE_FILE_LOCK)
            continue;

        // Compare to each locked file specified by the device.
        for (unsigned int s = 0; s < devices[i].device_type_data.file_lock->len; s++) {
            LOG_D("Comparing %s to %s", pathname, devices[i].device_type_data.file_lock->paths[s]);
            
            // TODO Treat locked filepaths as a list of paths.
            if (!strncmp(pathname, devices[i].device_type_data.file_lock->paths[s], strlen(pathname))) {
                return &devices[i];
            }
        }
    }

    return NULL;
}

char *
sunneed_device_get_dummy_file(const char *orig_path) {
    // Try to find the already-created locked file with that name.
    for (int i = 0; i < MAX_LOCKED_FILES; i++) {
        // TODO Hashmap or something.
        if (dummy_fd_map[i].fd != -1)
            if (strncmp(orig_path, dummy_fd_map[i].pathname, sizeof(dummy_fd_map[i].pathname)) == 0)
                return dummy_fd_map[i].dummypath;
    }
    
    char template[] = "locked_XXXXXX";
    int dummy;
    if ((dummy = mkstemp(template)) == -1) {
        LOG_E("Error creating temp dummy file");
        return NULL;
    }

    LOG_I("Created dummy file '%s'", template);
    
    for (int i = 0; i < MAX_LOCKED_FILES; i++) {
        if (dummy_fd_map[i].fd == -1) {
            dummy_fd_map[i].fd = dummy;            
            strncpy(dummy_fd_map[i].pathname, orig_path, sizeof(dummy_fd_map[i].pathname));
            strncpy(dummy_fd_map[i].dummypath, template, sizeof(dummy_fd_map[i].dummypath));
            return dummy_fd_map[i].dummypath;
        }
    }

    return NULL;
}
