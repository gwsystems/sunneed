#ifndef _SUNNEED_DEVICE_H_
#define _SUNNEED_DEVICE_H_

struct sunneed_device {
    void *dlhandle;
    int handle;
    char *identifier;
    void *(*get)(void *);
    double (*power_consumption)(void *);
    bool is_linked;
};

#endif
