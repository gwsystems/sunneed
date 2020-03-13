#ifndef _SUNNEED_DEVICE_H_
#define _SUNNEED_DEVICE_H_

struct sunneed_device {
    void *dlhandle;
    int handle;
    const char *identifier;
    void (**get)(void *);
    double (*power_consumption)(void *);
};

#endif
