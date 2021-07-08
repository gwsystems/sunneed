// This is here so `sunneed_core` can access it.
extern char sunneed_runtime_test_error[];

/*
 *********************************************************************
 * Place your collection of tests to run in the RUNTIME_TESTS macro. *
 *********************************************************************
 */
#include "sunneed_loader.h"
#define RUNTIME_TESTS { SUNNEED_RUNTIME_TESTS_LOADER }
