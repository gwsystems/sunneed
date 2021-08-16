<<<<<<< HEAD
#include "sunneed_device.h"

struct sunneed_device devices[MAX_DEVICES] = { { .is_ready = false } };
struct {
    bool init;
    char pathname[DUMMY_FILE_PATH_LEN]; // TODO Don't hardcode.
    char dummypath[DUMMY_FILE_PATH_LEN];
} dummy_map[MAX_LOCKED_FILES] = { { false, { '\0' }, { '\0' } } };

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
        if (dummy_map[i].init)
            if (strncmp(orig_path, dummy_map[i].pathname, sizeof(dummy_map[i].pathname)) == 0)
                return dummy_map[i].dummypath;
    }
    
    // We haven't made a dummy file for this yet.
    char template[] = "locked_XXXXXX";
    int dummy;
    if ((dummy = mkstemp(template)) == -1) {
        LOG_E("Error creating temp dummy file");
        return NULL;
    }

    LOG_I("Created dummy file '%s'", template);
    
    for (int i = 0; i < MAX_LOCKED_FILES; i++) {
        if (!dummy_map[i].init) {
            dummy_map[i].init = dummy;
            strncpy(dummy_map[i].pathname, orig_path, DUMMY_FILE_PATH_LEN);
            strncpy(dummy_map[i].dummypath, template, DUMMY_FILE_PATH_LEN);
            return dummy_map[i].dummypath;
        }
    }

    // Out of entires in dummy file table.
    LOG_E("Cannot create dummy file mapping for '%s'", orig_path);
    return NULL;
}

char *
get_path_from_dummy_path(const char *path) {
    for (int i = 0; i < MAX_LOCKED_FILES; i++)
	if (dummy_map[i].init)
	    if (strncmp(path, dummy_map[i].dummypath, sizeof(dummy_map[i].dummypath)) == 0)
		    return dummy_map[i].pathname;
    return NULL;
}
=======
#include "sunneed_device.h"

struct sunneed_device devices[MAX_DEVICES] = { { .is_ready = false } };
struct {
    bool init;
    char pathname[DUMMY_FILE_PATH_LEN]; // TODO Don't hardcode.
    char dummypath[DUMMY_FILE_PATH_LEN];
} dummy_map[MAX_LOCKED_FILES] = { { false, { '\0' }, { '\0' } } };

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
        if (dummy_map[i].init)
            if (strncmp(orig_path, dummy_map[i].pathname, sizeof(dummy_map[i].pathname)) == 0)
                return dummy_map[i].dummypath;
    }
    
    // We haven't made a dummy file for this yet.
    char template[] = "locked_XXXXXX";
    int dummy;
    if ((dummy = mkstemp(template)) == -1) {
        LOG_E("Error creating temp dummy file");
        return NULL;
    }

    LOG_I("Created dummy file '%s'", template);
    
    for (int i = 0; i < MAX_LOCKED_FILES; i++) {
        if (!dummy_map[i].init) {
            dummy_map[i].init = dummy;
            strncpy(dummy_map[i].pathname, orig_path, DUMMY_FILE_PATH_LEN);
            strncpy(dummy_map[i].dummypath, template, DUMMY_FILE_PATH_LEN);
            return dummy_map[i].dummypath;
        }
    }

    // Out of entires in dummy file table.
    LOG_E("Cannot create dummy file mapping for '%s'", orig_path);
    return NULL;
}
>>>>>>> origin/Ryan
