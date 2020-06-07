#include "test.h"

#include <stdio.h>
#include <stdlib.h>

MunitResult test_overlay_pid_is_same_as_process_pid(const MunitParameter params[], void *data);

static MunitTest overlay_tests[] = {
    MUNIT_BASIC_TESTCASE(test_overlay_pid_is_same_as_process_pid),
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite overlay_suite = {
    "/overlay",
    overlay_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};
