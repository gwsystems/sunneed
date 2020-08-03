#include "sunneed_loader.h"

/** 
 * Assign to the `device_type_data` member of a device struct, choosing the correct union member based on what has been assigned
 * to `device_type_kind`.
 */
static int
assign_device_type_data_field(struct sunneed_device *dev, void *data) {
    // We're gonna dereference a void pointer. Try not to worry about it.
    switch (dev->device_type_kind) {
        case DEVICE_TYPE_FILE_LOCK: ;
            // Copy each string from the data's `paths`.
            struct sunneed_device_type_file_lock *file_lock = (struct sunneed_device_type_file_lock *)data;
            dev->device_type_data.file_lock = (struct sunneed_device_type_file_lock *)malloc(sizeof(struct sunneed_device_type_file_lock) + sizeof(char *) * file_lock->len);
            for (unsigned int i = 0; i < file_lock->len; i++) { 
                // Copy the string, adding a null terminator.
                size_t len = strlen(file_lock->paths[i]);
                dev->device_type_data.file_lock->paths[i] = malloc(len + 1);
                strncpy(dev->device_type_data.file_lock->paths[i], file_lock->paths[i], len);
                dev->device_type_data.file_lock->paths[i][len] = '\0';
            }
            break;

        default:
            LOG_E("Invalid device kind %d", dev->device_type_kind);
            return 1;
    }

    return 0;
}

static bool
is_object_file(char *path) {
    size_t len = strlen(path);
    if (strncmp(path + len - OBJ_EXTENSION_LEN, OBJ_EXTENSION, len) == 0)
        return true;
    else
        return false;
}

static int
load_device(const char *device_path, const char *device_name, int handle, struct sunneed_device *dev) {
    int retval = 0;

    void *sym;

    // Load the object.
    void *dlhandle = dlopen(device_path, RTLD_LAZY | RTLD_LOCAL);
    if (!dlhandle) {
        LOG_E("Error loading device from '%s': %s", device_name, dlerror());
        return 1;
    }

    sym = dlsym(dlhandle, "device_flags");
    if (!sym) {
        LOG_E("Failed to load device flags: %s", dlerror()); 
        retval = 1;
        goto end;
    }
    unsigned int flags = *(unsigned int *)sym;

    // Set up device flags.
    bool silent_fail = false;
    if (flags) {
        silent_fail = flags & SUNNEED_DEVICE_FLAG_SILENT_FAIL;      
    }

    char *identifier = malloc(DEVICE_IDENTIFIER_LEN);
    if (!identifier) {
        if (!silent_fail) LOG_E("Failed to allocate memory for device name.");
        retval = 1;
        goto end;
    }

    sym = dlsym(dlhandle, "device_type_kind");
    if (!sym) {
        if (!silent_fail) LOG_E("Getting `device_type_kind` failed: %s", dlerror());
        retval = 1;
        goto end;
    }
    enum sunneed_device_type kind = *(enum sunneed_device_type *)sym;
    
    // Write to struct.
    *dev = (struct sunneed_device) {
            .is_ready = false,
            .handle = handle,
            .identifier = identifier,
            .device_type_kind = kind
    };
    strncpy(dev->identifier, device_name, DEVICE_IDENTIFIER_LEN);
    dev->identifier[DEVICE_IDENTIFIER_LEN - 1] = '\0';

    sym = dlsym(dlhandle, "init");
    if (!sym) {
        if (!silent_fail) LOG_E("Getting `init` failed: %s", dlerror());
        retval = 1;
        goto end;
    }
    int (*init)(void) = (int (*)(void))sym;

    // Call `init`, failing on nonzero.
    int init_val = init();
    if (init_val != 0) {
        if (!silent_fail && init_val > 0) LOG_E("`init` failed with %d", init_val);
        retval = 1;
        goto end;
    } 

    // Set up device type data.
    sym = dlsym(dlhandle, "get_device_type_data");
    if (!sym) {
        if (!silent_fail) LOG_E("Getting `get_device_type_data` failed: %s", dlerror());
        retval = 1;
        goto end;
    }

    // Obtain the pointer to the device type data.
    void *data = ((void *(*)(void))sym)();
    if (!data) {
        if (!silent_fail) LOG_E("No device type data returned!");
        retval = 1;
        goto end;
    }

    // Write that data to the appropriate union field.
    if (assign_device_type_data_field(dev, data)) {
        if (!silent_fail) LOG_E("Failed to assign to device type data field.");
        retval = 1;
        goto end;
    }

    // Hooray!
    dev->is_ready = true;

end:
    dlclose(dlhandle);

    // Clean up if error.
    if (retval != 0) {
        if (identifier) free(identifier);
    }

    return retval;
}

/* Loads all objects in the device directory as sunneed devices, storing them in the `target` array. */
int
sunneed_load_devices(struct sunneed_device *target) {
    int ret = 0, res;

    DIR *dir = opendir("build/device");
    struct dirent *ent;

    if (!dir) {
        LOG_E("Failed to open devices directory");
        ret = 1;
        goto end;
    }

    unsigned int device_count = 0;
    while ((ent = readdir(dir)) != NULL) {
        if (strlen(ent->d_name) <= OBJ_EXTENSION_LEN)
            // Can't be a real filename.
            continue;

        // Strip the `.so` from the path to get the device name.
        char device_name[DEVICE_PATH_LEN];
        strncpy(device_name, ent->d_name, strlen(ent->d_name) - OBJ_EXTENSION_LEN);
        device_name[strlen(ent->d_name) - OBJ_EXTENSION_LEN] = '\0';
            
        char device_path[DEVICE_PATH_LEN] = "build/device/";
        strncat(device_path, ent->d_name, DEVICE_PATH_LEN);

        if (is_object_file(device_path)) {
            if ((res = load_device(device_path, device_name, device_count++, target)) != 0)
                continue;

            LOG_I("Loaded device '%s'", device_name);

            target++;
        }
    }

end:
    closedir(dir);

    return ret;
}

#ifdef TESTING

int
TEST_load_device(void) {
    int res;

    struct sunneed_device dev;  
    if ((res = load_device("build/device/test_file_lock.so", "test", 0, &dev)) != 0)
        return set_sunneed_error(1, "`load_device` failed: %d", res);

    if (dev.handle != 0)
        return set_sunneed_error(2, "invalid handle %d", dev.handle);
    
    if (strcmp(dev.identifier, "test") != 0)
        return set_sunneed_error(3, "incorrect identifier '%s'", dev.identifier);

    if (strcmp(dev.device_type_data.file_lock->paths[0], TEST_FILE_LOCK_FILE_PATH) != 0)
        return set_sunneed_error(4, "wrong file lock path '%s'", dev.device_type_data.file_lock->paths[0]);

    return 0;
}

int
TEST_load_broken_device(void) {
    int res;

    struct sunneed_device dev;  
    if ((res = load_device("build/device/test_broken.so", "test", 0, &dev)) == 0)
        // Should fail.
        return set_sunneed_error(1, "loading broken device didn't fail");

    return 0;
}

#endif
