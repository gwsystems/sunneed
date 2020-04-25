#include "munit/munit.h"

#define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__) 

#define MUNIT_BASIC_TESTCASE(FUNC) \
    { (char*) ("/" #FUNC), FUNC, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
