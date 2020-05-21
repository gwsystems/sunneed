#include "sunneed_core.h"

struct sunneed_pip pip;

void *(*worker_thread_functions[])(void *) = {
    sunneed_proc_monitor,
    NULL
};

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

    int worker_thread_count = 0;
    for (void *(**cur)(void *) = worker_thread_functions; *cur != NULL; cur++)
        worker_thread_count++;

    pthread_t worker_threads[worker_thread_count];

    LOG_I("Launching %d worker threads", worker_thread_count);

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    for (int i = 0; i < worker_thread_count; i++) {
        if ((ret = pthread_create(&worker_threads[i], &attr, worker_thread_functions[i], NULL)) != 0) {
            LOG_E("Failed to launch worker thread %d (error %d)", i, ret);
            return 1;
        };
    }

    if ((ret = sunneed_listen()) != 0) {
        LOG_E("sunneed listener encountered a fatal error. Exiting.");
        return 1;
    }

    return 0;
}
