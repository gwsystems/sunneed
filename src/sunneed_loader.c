#include "sunneed_loader.h"

/** 
 * Assign to the `device_type_data` member of a device struct, choosing the correct union member based on what has been assigned
 * to `device_type_kind`.
 */
static int
assign_device_type_data_field(struct sunneed_device *dev, void *data) {
    switch (dev->device_type_kind) {
        case DEVICE_TYPE_FILE_LOCK: ;
            // This is just to get the size of the `files` field. 
            size_t fieldsize = sizeof(((struct sunneed_device_type_file_lock *)0)->files);
            if (sizeof(*data) > fieldsize) {
                LOG_E("Data for device '%s' is too large", dev->identifier);
                return 1;
            }

            strncpy(dev->device_type_data.file_lock.files, ((struct sunneed_device_type_file_lock *)data)->files, fieldsize);
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
    // Load the object.
    void *dlhandle = dlopen(device_path, RTLD_LAZY | RTLD_LOCAL);
    if (!dlhandle) {
        LOG_E("Error loading device from '%s': %s", device_name, dlerror());
        return 1;
    }

    unsigned int (*flags)(void) = dlsym(dlhandle, "device_flags");

    bool silent_fail = false;
    if (flags) {
        silent_fail = flags() & SUNNEED_DEVICE_FLAG_SILENT_FAIL;      
    }

    // Initialize the device instance.
    bool err = false;
    *dev = (struct sunneed_device) {
            .dlhandle = dlhandle,
            .handle = handle,
            .identifier = malloc(DEVICE_PATH_LEN),
            .get = dlsym(dlhandle, "get"),
            .power_consumption = dlsym(dlhandle, "power_consumption"),
            .is_linked = false
    };
    strncpy(dev->identifier, device_name, strlen(device_name));

    // Set up device type data.
    void *kindsym = dlsym(dlhandle, "device_type_kind");
    if (kindsym) {
        dev->device_type_kind = *(enum sunneed_device_type *)kindsym;

        void *(*data_fn)(void) = dlsym(dlhandle, "get_device_type_data");
        if (data_fn) {
            void *data = data_fn();
            if (assign_device_type_data_field(dev, data))
                err = true;
        } else err = true;
    } else err = true;

    // TODO Use gotos for handling different kinds of errors.
    // Check for errors during loading.
    if (!sunneed_device_is_linked(dev) || err) {
        if (!silent_fail)
            LOG_E("Error linking device '%s': %s", device_name, dlerror());
        return 1;
    }

    dev->is_linked = true;

    return 0;
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

            LOG_I("Loaded device \"%s\"", device_name);

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
    if ((res = load_device("build/device/test.so", "test", 0, &dev)) != 0)
        return 1;

    if (dev.handle != 0)
        return 2;
    
    if (strcmp(dev.identifier, "test") != 0)
        return 3;

    if (dev.power_consumption(NULL) != 0)
        return 4;

    if (strcmp(dev.get(NULL), TEST_DEVICE_OUTPUT) != 0)
        return 5;

    return 0;
}

int
TEST_load_broken_device(void) {
    int res;

    struct sunneed_device dev;  
    if ((res = load_device("build/device/test_broken.so", "test", 0, &dev)) == 0)
        // Should fail.
        return 1;

    return 0;
}

#endif
