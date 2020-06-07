#include "overlay.h"

MunitResult test_overlay_pid_is_same_as_process_pid(const MunitParameter params[], void *data) {
    // TODO Don't hardcode output directories.
    FILE *out = popen("build/run-with-overlay build/overlay_tester", "r");
    if (!out) {
        fprintf(stderr, "Failed to run overlay_tester command\n");
        return MUNIT_ERROR;
    }

    const size_t buflen = 32;
    char buffer[2][buflen];

    int i = 0;
    while (fgets(buffer[i], buflen, out) != NULL && i < 2) {
        // Strip trailing newline.
        strtok(buffer[i], "\n");
        if (!strncmp(buffer[i], "PID ", 4))
            i++;
    }
    if (i != 2) {
        fprintf(stderr, "Not enough PID lines read\n");
        return MUNIT_ERROR;
    }

    munit_assert_string_equal(buffer[0], buffer[1]);
}
