#include "../protobuf/c/device.pb-c.h"

#include "../protobuf/c/server.pb-c.h"
#include "../shared/sunneed_ipc.h"
#include "../shared/sunneed_files.h"

#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/reqrep0/req.h>

#define PACK_AND_SEND(req)                                          \
    {                                                               \
        nng_msg *msg;                                               \
        int req_len = sunneed_request__get_packed_size(&req);       \
        void *buf = malloc(req_len);                                \
        sunneed_request__pack(&req, buf);                           \
        SUNNEED_NNG_TRY(nng_msg_alloc, != 0, &msg, req_len);        \
        SUNNEED_NNG_TRY(nng_msg_insert, != 0, msg, buf, req_len);   \
        SUNNEED_NNG_TRY(nng_sendmsg, != 0, sunneed_socket, msg, 0); \
        free(buf);                                                  \
        nng_msg_free(msg);                                          \
    }

#define FATAL(CODE, FMT, ...)                                          \
    {                                                                  \
        fprintf(stderr, "fatal (%d): " FMT "\n", CODE, ##__VA_ARGS__); \
        exit(CODE);                                                    \
    }

typedef unsigned int sunneed_device_handle_t;

int
sunneed_client_init(const char *name);

char *
sunneed_client_fetch_locked_file_path(const char *pathname);

int
sunneed_client_check_locked_file(const char *pathname);

int
sunneed_client_disconnect(void);

int
sunneed_client_on_locked_path_open(int i, char *pathname, int fd);

void
sunneed_client_debug_print_locked_path_table(void);
