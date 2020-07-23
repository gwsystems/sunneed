// For testing purposes.
#define CAMERA_PATH "camera"

#define SUNNEED_DEVICE_FLAG_SILENT_FAIL (1 << 0)

enum sunneed_device_type {
    DEVICE_TYPE_FILE_LOCK = 1
};

struct sunneed_device_type_file_lock {
    // Semicolon-separated list of files to lock on.
    char files[1024];
};
