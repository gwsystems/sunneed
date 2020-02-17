#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "sunneed.h"
#include "sunneed_pip.h"
#include "sunneed_listener.h"
#include "log.h"

#define HELP_TEXT \
    APP_NAME ": enforce power usage policies\n" \
    "\nUSAGE\n" \
    "%s OPTIONS\n" \
    "\nOPTIONS\n" \
    "\t-h --help    Show this help.\n" \
    "\n"
