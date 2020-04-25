#include "nng.h"

static void nng_fatal(const char *func, int rv) {
    fprintf(stderr, "%s: %s\n", func, nng_strerror(rv));
}

MunitResult test_nng_try_macro(const MunitParameter params[], void *data) {
    const size_t msg_sz = 16;

    SUNNEED_NNG_SET_ERROR_REPORT_FUNC(nng_fatal);

    nng_msg *msg;
    SUNNEED_NNG_TRY(nng_msg_alloc, !=0, &msg, msg_sz);
    munit_assert_size(nng_msg_len(msg), ==, msg_sz);

    nng_msg_free(msg);

    return MUNIT_OK;
}

MunitResult test_nng_try_set_macro(const MunitParameter params[], void *data) {
    const size_t msg_sz = 16;

    SUNNEED_NNG_SET_ERROR_REPORT_FUNC(nng_fatal);

    nng_msg *msg;
    int ret = 1;
    SUNNEED_NNG_TRY_SET(nng_msg_alloc, ret, !=0, &msg, msg_sz);
    munit_assert_size(nng_msg_len(msg), ==, msg_sz);

    munit_assert_int(ret, ==, 0);

    nng_msg_free(msg);

    return MUNIT_OK;
}
