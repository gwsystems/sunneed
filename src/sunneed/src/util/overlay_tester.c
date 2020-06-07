// Example of a program that would run on a sunneed system. This program should be run with `LD_PRELOAD` overlaying
//  our custom functions. This program doesn't reference sunneed in any way; the design is that communication
//  should be automatic.
//

#include <stdio.h>
#include <unistd.h>

int main(void) {
    printf("PID %d\n", getpid());
    return 0;
}
