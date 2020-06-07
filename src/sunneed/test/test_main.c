#include "munit/munit.h"

#include "test.gen.h"

SUITES_ARRAY(sub_suites);

static const MunitSuite main_suite = {
    (char*) "/sunneed",
    NULL,
    sub_suites,
    1,
    MUNIT_SUITE_OPTION_NONE
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
int main(int argc, const char *argv[]) {
    return munit_suite_main(&main_suite, NULL, argc, argv);
}
#pragma GCC diagnostic pop
