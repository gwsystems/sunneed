#include "../protobuf/c/device.pb-c.h"

#include "../protobuf/c/server.pb-c.h"
#include "../shared/sunneed_ipc.h"
#include "../shared/sunneed_files.h"

#include <stdbool.h>

#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/reqrep0/req.h>

#define FATAL(CODE, FMT, ...)                                          \
    {                                                                  \
        fprintf(stderr, "fatal (%d): " FMT "\n", CODE, ##__VA_ARGS__); \
        exit(CODE);                                                    \
    }

#define client_printf(FMT, ...) \
    printf("\e[38;5;240mclient:\e[0m " FMT, ##__VA_ARGS__)

typedef unsigned int sunneed_device_handle_t;

int
sunneed_client_init(const char *name);

char *
sunneed_client_fetch_locked_file_path(const char *pathname);

int
sunneed_client_check_locked_file(const char *pathname);

bool
sunneed_client_fd_is_locked(int fd);

ssize_t
sunneed_client_remote_write(int fd, const void *data, size_t n_bytes);

int
sunneed_client_disconnect(void);

int
sunneed_client_on_locked_path_open(int i, char *pathname, int fd);

void
sunneed_client_debug_print_locked_path_table(void);
