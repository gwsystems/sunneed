#include "sunneed_client.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

struct {
    char *path;
    int fd;
} locked_paths[MAX_LOCKED_FILES] = { { NULL, 0 } };

nng_socket sunneed_socket;

static void
nngfatal(const char *func, int rv) {
    fprintf(stderr, "%s: %s\n", func, nng_strerror(rv));
    exit(1);
}

/** 
 * Packs the given SunneedRequest into an NNG message and sends it to sunneed. If any failures occur in the packing
 *  or sending processes, this client will crash with a fatal error.
 */
static void
send_request(SunneedRequest *req) {
    nng_msg *msg;                                               

    int req_len = sunneed_request__get_packed_size(req);
    void *buf = malloc(req_len);
    if (!buf)
        FATAL(-1, "unable to allocate buffer for request");
    sunneed_request__pack(req, buf);                           

    SUNNEED_NNG_TRY(nng_msg_alloc, != 0, &msg, req_len);        
    SUNNEED_NNG_TRY(nng_msg_insert, != 0, msg, buf, req_len);   
    SUNNEED_NNG_TRY(nng_sendmsg, != 0, sunneed_socket, msg, 0);

    free(buf);
    nng_msg_free(msg);                                         
}

static SunneedResponse *
receive_response(SunneedResponse__MessageTypeCase message_type) {
    nng_msg *reply;
    SUNNEED_NNG_TRY(nng_recvmsg, != 0, sunneed_socket, &reply, 0);

    size_t msg_len = nng_msg_len(reply);
    SUNNEED_NNG_MSG_LEN_FIX(msg_len);
    SunneedResponse *resp = sunneed_response__unpack(NULL, msg_len, nng_msg_body(reply));

    if (resp->status != 0) {
        return NULL;
    } else if (resp->message_type_case != message_type) {
        FATAL(-1, "incorrect message type received (expected %d, got %d)", message_type, resp->status);
    }

    nng_msg_free(reply);
    return resp;
}

int
sunneed_client_init(const char *name) {
    SUNNEED_NNG_SET_ERROR_REPORT_FUNC(nngfatal);
    SUNNEED_NNG_TRY(nng_req0_open, != 0, &sunneed_socket);
    SUNNEED_NNG_TRY(nng_dial, != 0, sunneed_socket, SUNNEED_LISTENER_URL, NULL, 0);

    // Register this client with sunneed.
    SunneedRequest req = SUNNEED_REQUEST__INIT;
    req.message_type_case = SUNNEED_REQUEST__MESSAGE_TYPE_REGISTER_CLIENT;
    RegisterClientRequest register_req = REGISTER_CLIENT_REQUEST__INIT;
    register_req.name = malloc(strlen(name) + 1);
    if (!register_req.name) {
        fprintf(stderr, "failed to allocate memory for client name\n");
        return -1;
    }
    strncpy(register_req.name, name, strlen(name) + 1);
    req.register_client = &register_req;
    send_request(&req);
    free(register_req.name);

    // Check the response.
    SunneedResponse *resp = receive_response(SUNNEED_RESPONSE__MESSAGE_TYPE_REGISTER_CLIENT);
    for (size_t i = 0; i < resp->register_client->n_locked_paths; i++) {
        locked_paths[i].path = malloc(strlen(resp->register_client->locked_paths[i]) + 1);
        strcpy(locked_paths[i].path, resp->register_client->locked_paths[i]);
        printf("Registered locked path '%s'\n", locked_paths[i].path);
    }
    sunneed_response__free_unpacked(resp, NULL);

    return 0;
}

/** Allocate a string containing the path of the dummy file corresponding to the given path. */
char *
sunneed_client_fetch_locked_file_path(const char *pathname) {
    // TODO Check socket opened.

    SunneedRequest req = SUNNEED_REQUEST__INIT;
    req.message_type_case = SUNNEED_REQUEST__MESSAGE_TYPE_OPEN_FILE;
    OpenFileRequest open_file_req = OPEN_FILE_REQUEST__INIT;
    open_file_req.path = malloc(strlen(pathname) + 1);
    if (!open_file_req.path) {
        FATAL(-1, "failed to allocated memory for path");
    }
    strncpy(open_file_req.path, pathname, strlen(pathname) + 1);
    req.open_file = &open_file_req;

    send_request(&req);
    free(open_file_req.path);

    // TODO Handle request of a path that isn't locked.
    
    SunneedResponse *resp = receive_response(SUNNEED_RESPONSE__MESSAGE_TYPE_OPEN_FILE);
    if (resp == NULL) {
        // TODO Gotos
        return 0;
    }

    printf("Opening dummy path '%s'\n", resp->open_file->path);
    char *path = malloc(strlen(resp->open_file->path) + 1);
    if (!path)
        FATAL(-1, "unable to allocate path");
    strcpy(path, resp->open_file->path);

    sunneed_response__free_unpacked(resp, NULL);

    return path;
}

int
sunneed_client_check_locked_file(const char *pathname) {
    for (int i = 0; i < MAX_LOCKED_FILES; i++) {
        if (locked_paths[i].path == NULL)
            return -1;
        else if (strncmp(pathname, locked_paths[i].path, strlen(pathname)) == 0)
            return i; 
    }

    return -1;
}

int
sunneed_client_on_locked_path_open(int i, char *pathname, int fd) {
    if (i < 0 || i >= MAX_LOCKED_FILES)
        FATAL(-1, "locked-file array index out of bounds");
    if (pathname == NULL)
        FATAL(-1, "pathname is null");
    if (fd <= 0)
        FATAL(-1, "illegal FD");

    locked_paths[i].path = pathname;
    locked_paths[i].fd = fd;

    return 0;
}

int
sunneed_client_disconnect(void) {
    // TODO Check socket opened.

    SunneedRequest req = SUNNEED_REQUEST__INIT;
    req.message_type_case = SUNNEED_REQUEST__MESSAGE_TYPE_UNREGISTER_CLIENT;
    UnregisterClientRequest unregister_req = UNREGISTER_CLIENT_REQUEST__INIT;
    req.unregister_client = &unregister_req;
    send_request(&req);

    SunneedResponse *resp = receive_response(SUNNEED_RESPONSE__MESSAGE_TYPE_GENERIC);
    if (resp == NULL) {
        // TODO Handle
        printf("Disconnect response was NULL\n");
    }
    sunneed_response__free_unpacked(resp, NULL);

    printf("Unregistered.\n");
    return 0;
}

void
sunneed_client_debug_print_locked_path_table(void) {
    printf("Client locked files: [\n");
    for (int i = 0; i < MAX_LOCKED_FILES; i++) {
        if (locked_paths[i].path != NULL)
            printf("    FD %d : '%s'\n", locked_paths[i].fd, locked_paths[i].path);
    }
    printf("]\n");
}
