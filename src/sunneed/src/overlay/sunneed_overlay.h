// Symbols to overlay on top of existing programs via LD_PRELOAD.

#include <stdio.h>
#include <unistd.h>

// This will be run as soon as the library is linked.
void __attribute__((constructor)) on_load();
