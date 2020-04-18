#include "sunneed_core.h"

struct sunneed_pip pip;

void
sunneed_init(void) {
    pip = pip_info();
}

int
main(int argc, char *argv[]) {
    int opt;
    extern int optopt;

    while ((opt = getopt(argc, argv, ":h")) != -1) {
        switch (opt) {
            case 'h':
                printf(HELP_TEXT, argv[0]);
                exit(0);
            case '?':
                fprintf(stderr, "%s: illegal option -%c\n", APP_NAME, optopt);
                exit(1);
            case ':':
                fprintf(stderr, "%s: expected argument for option -%c\n", APP_NAME, optopt);
                exit(1);
        }
    }

    LOG_I("sunneed is initializing...");

    sunneed_init();

    LOG_I("Acquired PIP: %s", pip.name);

    int ret;

    if ((ret = sunneed_listen()) != 0) {
        LOG_E("sunneed listener encountered a fatal error. Exiting.");
        return 1;
    }

    return 0;
}
