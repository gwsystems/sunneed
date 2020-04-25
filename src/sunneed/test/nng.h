#include "test.h"

#include <stdio.h>

#include <nng/nng.h>

#include "../src/shared/sunneed_ipc.h"

MunitResult test_nng_try_macro(const MunitParameter params[], void *data);
MunitResult test_nng_try_set_macro(const MunitParameter params[], void *data);

static MunitTest nng_tests[] = {
    MUNIT_BASIC_TESTCASE(test_nng_try_macro),
    MUNIT_BASIC_TESTCASE(test_nng_try_set_macro),
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite nng_suite = {
    "/nng",
    nng_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};
