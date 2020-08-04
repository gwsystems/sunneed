#ifndef _SUNNEED_TEST_H_
#define _SUNNEED_TEST_H_

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// Imagine using non-"power of 2" numbers lmao.
#define SUNNEED_RUNTIME_TEST_ERROR_DESC_BUFFER_LENGTH 1024

/**
 * A convenience method for testing.
 * Writes the given message to the error description buffer and returns the given value.
 *
 * Example usage:
 *
 *    if (my_test_variable != 0)
 *        return set_sunneed_error(1, "test condition failed, variable has value %d", my_test_variable);
 *
 */
int set_sunneed_error(int ret, const char *format, ...);

#endif
