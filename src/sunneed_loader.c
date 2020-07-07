#include "sunneed_loader.h"

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

    // Initialize the device instance.
    *dev = (struct sunneed_device) {
            .dlhandle = dlhandle,
            .handle = handle,
            .identifier = malloc(DEVICE_PATH_LEN),
            .get = dlsym(dlhandle, "get"),
            .power_consumption = dlsym(dlhandle, "power_consumption"),
            .is_linked = false
    };
    strncpy(dev->identifier, device_name, strlen(device_name));

    if (!sunneed_device_is_linked(dev)) {
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
        device_name[strlen(ent->d_name) - OBJ_EXTENSION_LEN + 1] = '\0';
            
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
