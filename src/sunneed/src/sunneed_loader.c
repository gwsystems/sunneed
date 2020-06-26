#include "sunneed_loader.h"

static bool
is_object_file(char *path) {
    size_t len = strlen(path);
    if (strncmp(path + len - OBJ_EXTENSION_LEN, OBJ_EXTENSION, len) == 0)
        return true;
    else
        return false;
}

/* Loads all objects in the device directory as sunneed devices, storing them in the `target` array. */
int
sunneed_load_devices(struct sunneed_device *target) {
    int ret = 0;

    DIR *dir = opendir("build/device");
    struct dirent *ent;

    if (!dir) {
        LOG_E("Failed to open devices directory");
        ret = 1;
        goto end;
    }

    unsigned int device_count = 0;
    while ((ent = readdir(dir)) != NULL) {
        char device_path[DEVICE_PATH_LEN] = "build/device/";
        strncat(device_path, ent->d_name, DEVICE_PATH_LEN);

        if (is_object_file(device_path)) {
            // Strip the `.so` from the path to get the device name.
            char device_name[DEVICE_PATH_LEN];
            strncpy(device_name, ent->d_name, strlen(ent->d_name) - OBJ_EXTENSION_LEN);
            device_name[strlen(ent->d_name) - OBJ_EXTENSION_LEN + 1] = '\0';

            // Load the object.
            void *dlhandle = dlopen(device_path, RTLD_LAZY | RTLD_LOCAL);
            if (!dlhandle) {
                LOG_E("Error loading device from '%s': %s", device_name, dlerror());
                continue;
            }

            // Initialize the device instance.
            struct sunneed_device dev = (struct sunneed_device) {
                .dlhandle = dlhandle,
                .handle = device_count,
                .identifier = malloc(DEVICE_PATH_LEN),
                .get = dlsym(dlhandle, "get"),
                .power_consumption = dlsym(dlhandle, "power_consumption"),
                .is_linked = false
            };
            strncpy(dev.identifier, device_name, strlen(device_name));

            if (!sunneed_device_is_linked(&dev)) {
                LOG_E("Error linking device '%s': %s", device_name, dlerror());
                continue;
            }

            dev.is_linked = true;

            // Allocate space in the return array.
            target[device_count++] = dev;

            LOG_I("Loaded device \"%s\"", dev.identifier);
        }
    }

end:
    closedir(dir);

    return ret;
}
