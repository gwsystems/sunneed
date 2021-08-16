// Symbols to overlay on top of existing programs via LD_PRELOAD.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <dlfcn.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../client/sunneed_client.h"
#include "../shared/sunneed_device_type.h"

/** 
 * Call the overridden function from within an overlay function. This is done via the RTLD_NEXT flag, which instructs
 *  dlsym to find the next match for the symbol in libraries loaded after the current one.
 *
 * Arguments:
 * RETVAR is a variable to assign the return value of the function to.
 * NAME is the name of the function (not in a string).
 * RETURN_TYPE is the type signature for the return value of the function.
 * ARGS should be the argument names passed to the current overlay function, wrapped in parentheses. For example,
 *  calling the base `open(const char *path, int flags)`, the ARGS should be in the form `(path, flags)`.
 * Finally, the varargs is the type signatures of the arguments, comma-separated. These should end up lining up with
 *  the types of the args passed as ARGS.
 *
 * To illustrate, let's return to our example using `open`. We'll declare an `int` to hold the return value, and then
 *  invoke `SUPER`.
 *
 *      int ret;
 *      SUPER(ret, open, int, (path, flags), const char *, int);
 *
 * TODO The potential pitfall here is that we perform a `dlsym` lookup every single time we use `SUPER`. We should
 *  either implement some caching system, or make sure that `dlsym` is caching these values under the hood.
 */
#define SUPER(RETVAR, NAME, RETURN_TYPE, ARGS, ...) { \
    RETURN_TYPE (*_base)(__VA_ARGS__); \
    _base = dlsym(RTLD_NEXT, # NAME); \
    RETVAR = (*_base) ARGS; \
}


// This will be run as soon as the library is linked (program start).
void __attribute__((constructor)) on_load();

// ...and likewise, this one when unlinked (program end).
void __attribute__((destructor)) on_unload();

int
open(const char *pathname, int flags, mode_t mode);

ssize_t
write(int fd, const void *buf, size_t count);

int
socket(int domain, int type, int protocol);

int
connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

ssize_t
send(int sockfd, const void *buf, size_t len, int flags);


