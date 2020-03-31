#include "sunneed_listener.h"

extern struct sunneed_device devices[];

// Control flow:
// When a new client (identified by pipe ID) connects, we register it in the clint_states array.
// Once registered, clients can send requests over NNG. Requests consist of a token word that describes the request
//  being made, optionally followed by a newline character delimiting the arguments for that command.
// The client will have to send some notification in order to unregister; I don't think we can tell if a pipe
//  closed.
static struct client_state {
    int index;
    bool is_active;
    nng_pipe pipe;
};

struct client_state client_states[SUNNEED_MAX_IPC_CLIENTS];

// TODO This is probably slow -- O(n) lookup for every request made.
static struct client_state *get_client_state_by_pipe_id(int pipe_id) {
    for (int i = 0; i < SUNNEED_MAX_IPC_CLIENTS; i++)
        if (nng_pipe_id(client_states[i].pipe) == pipe_id)
            return &client_states[i];
    return NULL;
}

static struct client_state *register_client_state(nng_pipe pipe) {
    int idx;
    for (idx = 0; idx < SUNNEED_MAX_IPC_CLIENTS; idx++)
        if (!client_states[idx].is_active)
            break;
    if (idx == SUNNEED_MAX_IPC_CLIENTS)
        return NULL;

    client_states[idx] = (struct client_state){
        .index = idx,
        .is_active = true,
        .pipe = pipe
    };

    return &client_states[idx];
}

static int unregister_client_state(nng_pipe pipe) {
    int idx;
    for (idx = 0; idx < SUNNEED_MAX_IPC_CLIENTS; idx++)
        if (nng_pipe_id(pipe) != -1 && nng_pipe_id(client_states[idx].pipe) == nng_pipe_id(pipe))
            break;
    

    // TODO Unregister.
}

static int serve_get_handle(const char *identifier) {
    static int handle_cur = 0;

    char filename[SUNNEED_DEVICE_PATH_MAX_LEN];

    int len;
    if ((len = snprintf(filename, SUNNEED_DEVICE_PATH_MAX_LEN, "build/devices/%s.so", identifier) 
                > SUNNEED_DEVICE_PATH_MAX_LEN)) {
        LOG_E("sunneed error: device name '%s' is too long", identifier);
        return -1;
    }

    void *dlhandle = dlopen(filename, RTLD_LOCAL);

    devices[handle_cur] = (struct sunneed_device) {
        .dlhandle = dlhandle,
        .handle = handle_cur,
        .identifier = identifier,
        // TODO Check for dlsym error
        .get = dlsym(dlhandle, "get"),
        .power_consumption = dlsym(dlhandle, "power_consumption")
    };

    return handle_cur++;  
}

static void report_nng_error(const char *func, int rv) {
    LOG_E("nng error: (%s) %s", func, nng_strerror(rv));
}

int sunneed_listen(void) {
    SUNNEED_NNG_SET_ERROR_REPORT_FUNC(report_nng_error);

    // Initialize client states.
    for (int i = 0; i < MAX_TENANTS; i++) {
        client_states[i] = (struct client_state){
            .is_active = false,
            // TODO Why do I need to cast this...
            .pipe = (nng_pipe)NNG_PIPE_INITIALIZER
        };
    }

    nng_socket sock;

    LOG_I("Starting listener loop...");

    // Make a socket and attach it to the sunneed URL.
    SUNNEED_NNG_TRY_RET(nng_rep0_open, !=0, &sock);
    SUNNEED_NNG_TRY_RET(nng_listen, <0, sock, SUNNEED_LISTENER_URL, NULL, 0);

    // Await messages.
    for (;;) {
        nng_msg *msg;

        SUNNEED_NNG_TRY_RET(nng_recvmsg, !=0, sock, &msg, NNG_FLAG_ALLOC);

        // TODO They claim nng_msg_get_pipe() returns -1 on error, but its return type is nng_pipe, which can't 
        //  be compared to an integer.
        nng_pipe pipe = nng_msg_get_pipe(msg);

        int pipe_id;
        SUNNEED_NNG_TRY_RET_SET(nng_pipe_id, pipe_id, ==-1, pipe);

        // Create and allocate the reply message here for convenience.
        // If insertions are made, nng will automatically reallocate the body pointer with more storage to fit the
        //  data.
        nng_msg *reply;
        SUNNEED_NNG_TRY_RET(nng_msg_alloc, !=0, &reply, SUNNEED_MESSAGE_DEFAULT_BODY_SZ);

        char *buf = nng_msg_body(msg);
        LOG_D("From pipe %d: %s", pipe_id, buf);

        struct client_state *msg_client_state = NULL;

        // Check for registration request.
        if (strncmp(buf, SUNNEED_IPC_REQ_REGISTER, strlen(buf)) == 0) {
            // Register as a new client.
            if ((msg_client_state = register_client_state(pipe)) == NULL) {
                LOG_W("Registration failed for pipe %d", pipe_id);
                goto end;
            }
            LOG_D("Registered pipe %d as client %d", pipe_id, msg_client_state->index);
            SUNNEED_NNG_TRY_RET(nng_msg_insert, !=0, reply, SUNNEED_IPC_REP_SUCCESS, strlen(SUNNEED_IPC_REP_SUCCESS));
            SUNNEED_NNG_TRY_RET(nng_sendmsg, !=0, sock, reply, 0);
            goto end;
        }

        // Find the pipe's associated client state (unless we already found it as a part of registration).
        if (!msg_client_state && (msg_client_state = get_client_state_by_pipe_id(pipe_id)) == NULL) {
            // This client has not registered!
            LOG_W("Received message from %d, who is not registered.", pipe_id);
            goto end;
        }

        // Interpret command tokens.
        if (strncmp(SUNNEED_IPC_REQ_UNREGISTER, buf, strlen(SUNNEED_IPC_REQ_UNREGISTER)) == 0) {
            // Unregister client.
        } else if (strncmp(SUNNEED_IPC_TEST_REQ_STR, buf, strlen(SUNNEED_IPC_TEST_REQ_STR)) == 0) {
            // Handle IPC test request.
            SUNNEED_NNG_TRY_RET(nng_msg_insert, !=0, reply, SUNNEED_IPC_TEST_REP_STR, strlen(SUNNEED_IPC_TEST_REP_STR));
            SUNNEED_NNG_TRY_RET(nng_sendmsg, !=0, sock, reply, 0);
        } else if (strncmp(SUNNEED_IPC_REQ_GET_DEVICE_HANDLE, buf, strlen(SUNNEED_IPC_REQ_GET_DEVICE_HANDLE)) == 0) {
            if (*(buf + strlen(SUNNEED_IPC_REQ_GET_DEVICE_HANDLE)) != '\n') {
                LOG_D("Request from pipe %d missing newline argument separator.", pipe_id);
                SUNNEED_NNG_TRY_RET(nng_msg_insert, !=0, reply, SUNNEED_IPC_REP_FAILURE, strlen(SUNNEED_IPC_REP_FAILURE));
                SUNNEED_NNG_TRY_RET(nng_sendmsg, !=0, sock, reply, 0);
                goto end;
            }

            char *device_name = buf + strlen(SUNNEED_IPC_REQ_GET_DEVICE_HANDLE) + 1;

            int handle;
            if ((handle = serve_get_handle(device_name)) < 0) {
                LOG_E("Failed to get handle for '%s'.", device_name);
                return 1;
            }

            // TODO No magic number for size.
            char response[64];
            snprintf(response, 64, "%s\n%d", SUNNEED_IPC_REP_RESULT, handle);
            SUNNEED_NNG_TRY_RET(nng_msg_insert, !=0, reply, response, strlen(response));
            SUNNEED_NNG_TRY_RET(nng_sendmsg, !=0, sock, reply, 0);
        }

end:
        nng_msg_free(reply);
        nng_msg_free(msg);
    }
}
