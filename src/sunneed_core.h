<<<<<<< HEAD
#ifndef _SUNNEED_CORE_H_
#define _SUNNEED_CORE_H_

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "log.h"
#include "shared/sunneed_pip_interface.h"
#include "sunneed.h"
#include "sunneed_listener.h"
#include "sunneed_proc.h"
#include "sunneed_loader.h"
#include "sunneed_device.h"

#define _HELP_TEXT_HEAD                         \
    APP_NAME ": enforce power usage policies\n" \
             "\nUSAGE\n"                        \
             "%s OPTIONS\n"                     \
             "\nOPTIONS\n"                      \
             "\t-h --help    Show this help.\n"
#define _HELP_TEXT_TAIL \
            "\n"

#ifdef TESTING
#define HELP_TEXT _HELP_TEXT_HEAD \
    "\t-c --testcase-count  Print out the number of runtime tests.\n" \
    "\t-t --run-test TEST   Run a runtime test by given its numerical ID.\n" \
    _HELP_TEXT_TAIL
#else
#define HELP_TEXT _HELP_TEXT_HEAD _HELP_TEXT_TAIL
#endif

#ifdef TESTING

#define MAX_TESTS_PER_SUITE 64

#endif

#endif
=======
#ifndef _SUNNEED_CORE_H_
#define _SUNNEED_CORE_H_

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "log.h"
#include "shared/sunneed_pip_interface.h"
#include "sunneed.h"
#include "sunneed_listener.h"
#include "sunneed_proc.h"
#include "sunneed_loader.h"
#include "sunneed_device.h"

#define _HELP_TEXT_HEAD                         \
    APP_NAME ": enforce power usage policies\n" \
             "\nUSAGE\n"                        \
             "%s OPTIONS\n"                     \
             "\nOPTIONS\n"                      \
             "\t-h --help    Show this help.\n"
#define _HELP_TEXT_TAIL \
            "\n"

#ifdef TESTING
#define HELP_TEXT _HELP_TEXT_HEAD \
    "\t-c --testcase-count  Print out the number of runtime tests.\n" \
    "\t-t --run-test TEST   Run a runtime test by given its numerical ID.\n" \
    _HELP_TEXT_TAIL
#else
#define HELP_TEXT _HELP_TEXT_HEAD _HELP_TEXT_TAIL
#endif

#ifdef TESTING

#define MAX_TESTS_PER_SUITE 64

#endif

#endif
>>>>>>> c1ce9be87ee40f52cd027a965adce8ad5d136a36
