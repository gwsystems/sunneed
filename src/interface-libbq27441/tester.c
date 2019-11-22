#include <stdlib.h>
#include <stdio.h>

#include "bq27441.h"

void exit_tester(void) {
    fprintf(stderr, "Exiting due to libbq27441 error...\n");
    exit(1);
}

void main(void) {
    if (bq27441_init(1) != 0)
        exit_tester();

    printf("If we got here, our device ID is corect.\n");
}
