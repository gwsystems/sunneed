#include "../client/sunneed_client.h"

#include <stdio.h>

int main(void) {
    int ret;

    if ((ret = sunneed_client_init("Drain Gang")) != 0)
        FATAL(ret, "failed to connect to sunneed");

    sunneed_device_handle_t handle;
    if ((ret = sunneed_client_get_device_handle("random", &handle)) != 0)
        FATAL(ret, "failed to get device handle");
    
    if ((ret = sunneed_client_disconnect()) != 0)
        FATAL(ret, "failed to disconnect");

    return 0;
}
