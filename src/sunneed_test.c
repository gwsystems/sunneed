#include "sunneed_test.h"

char sunneed_runtime_test_error[SUNNEED_RUNTIME_TEST_ERROR_DESC_BUFFER_LENGTH] = { '\0' };

int set_sunneed_error(int ret, const char *format, ...) {
    if (sunneed_runtime_test_error[0] != '\0') {
        fprintf(stderr, "The test failure message buffer has already been modified.\n"
                        "Something is messed up with the tests!\n");
        exit(1);
    }

    va_list ap;
    va_start(ap, format);
    vsnprintf(sunneed_runtime_test_error, SUNNEED_RUNTIME_TEST_ERROR_DESC_BUFFER_LENGTH, format, ap);
    va_end(ap);

    return ret;
}
