#include "sunneed_core.h"

extern struct sunneed_device devices[MAX_DEVICES];

struct sunneed_pip pip;

sunneed_worker_thread_result_t (*worker_thread_functions[])(void *) = {sunneed_proc_monitor, sunneed_quantum_worker, sunneed_stepperMotor_driver, sunneed_camera_driver, NULL};

void
handle_exit(void) {
    LOG_I("Sunneed exiting");
    LOG_I("\tKilling stepper motor");
    kill(sunneed_stepper_driver_pid, SIGTERM);
    LOG_I("\tKilling camera driver");
    kill(sunneed_camera_driver_pid, SIGTERM);
}

#ifdef TESTING

#include "sunneed_runtime_test_collection.h"

int (*runtime_tests[])(void) = RUNTIME_TESTS;

static unsigned int
testcase_count(void) {
    unsigned int testcases = 0;
    for (int (**cur)(void) = runtime_tests; *cur != NULL; cur++)
        testcases++; 
    // TODO Why minus one...
    return testcases - 1;
}

static int 
run_testcase(unsigned int testcase) {
    if (testcase >= testcase_count()) {
        LOG_E("Cannot run testcase #%d because it does not exist", testcase);
        return 1;
    }

    int ret = runtime_tests[testcase]();
    if (ret != 0) {
        fprintf(stderr, "Failure: %s (%d)\n", sunneed_runtime_test_error, ret);
        return ret;
    }

    return 0;
}
#endif

static int
spawn_worker_threads(void) {
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

    return 0;
}


void
sunneed_init(void) {
    atexit(handle_exit);
    pip = pip_info();
}

int
main(int argc, char *argv[]) {
    int opt;
    extern int optopt;

#ifdef LOG_PWR
    logfile_pwr = fopen("sunneed_pwr_log.txt", "w+");
#endif

#ifdef TESTING
    const char *optstring = ":ht:c";
#else
    const char *optstring = ":h";
#endif
    // TODO Long-form getopts.
    while ((opt = getopt(argc, argv, optstring)) != -1) {
	switch (opt) {
            case 'h':
                printf(HELP_TEXT, argv[0]);
                exit(0);
#ifdef TESTING
            case 't': ;
   		logfile = fopen("sunneed_log.txt", "w+");
                int testcase = strtol(optarg, NULL, 10);
                if (errno) {
                    LOG_E("Failed to parse testcase index: %s", strerror(errno));
                    return 1;
                }
                return run_testcase(testcase);
            case 'c':
                printf("%d\n", testcase_count());
                exit(0);
#endif
            case '?':
                fprintf(stderr, "%s: illegal option -%c\n", APP_NAME, optopt);
                exit(1);
            case ':':
                fprintf(stderr, "%s: expected argument for option -%c\n", APP_NAME, optopt);
                exit(1);
        }
    }

    int ret = 0;

    LOG_I("sunneed is initializing...");

    sunneed_init();

    LOG_I("Acquired PIP: %s", pip.name);

    LOG_I("Loading devices...");
    if ((ret = sunneed_load_devices(devices)) != 0) {
        LOG_E("Failed to load devices");
        ret = 1;
        goto end;
    }

    if ((ret = spawn_worker_threads()) != 0) {
        LOG_E("Error occurred while spawning worker threads");
        ret = 1;
        goto end;
    }
    if ((ret = sunneed_listen()) != 0) {
        LOG_E("sunneed listener encountered a fatal error. Exiting.");
        ret = 1;
        goto end;
    }

end:
    return 0;
}
