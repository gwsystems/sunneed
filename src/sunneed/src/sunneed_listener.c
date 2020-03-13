#include "sunneed_listener.h"

extern struct sunneed_device devices[];

// Control flow:
// When a new client (identified by pipe ID) connects, we register it in the clint_states array.
// Clients send state requests such as SUNNEED_IPC_REQ_GET_DEVICE_HANDLE, which are a single meessage 
//  consisting of the command text. sunneed assigns the corresponding value to the `state` field.
// The next message sent after a state request will be interpreted as the arguments for the `serve_*` function 
//  associated with that state.
// The client will have to send some notification in order to unregister; I don't think we can tell if a pipe
//  closed.
static struct client_state {
    int index;
    bool is_active;
    nng_pipe pipe;
    enum sunneed_client_ipc_state state;
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
        .pipe = pipe,
        .state = STATE_NONE
    };

    return &client_states[idx];
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
    // Initialize client states.
    for (int i = 0; i < MAX_TENANTS; i++) {
        client_states[i] = (struct client_state){
            .is_active = false,
            .state = STATE_NONE,
            // TODO Why do I need to cast this...
            .pipe = (nng_pipe)NNG_PIPE_INITIALIZER
        };
    }

    nng_socket sock;
    int rv;

    LOG_I("Starting listener loop...");

    // Make a socket and attach it to the sunneed URL.
    if ((rv = nng_rep0_open(&sock)) != 0) {
        report_nng_error("nng_rep0_open", rv);
        return 1;
    }
    if ((rv = nng_listen(sock, SUNNEED_LISTENER_URL, NULL, 0)) < 0) {
        report_nng_error("nng_listen", rv);
        return 1;
    }

    // Await messages.
    for (;;) {
        nng_msg *msg;

        if ((rv = nng_recvmsg(sock, &msg, NNG_FLAG_ALLOC)) != 0) {
            report_nng_error("nng_recvmsg", rv);
            return 1;
        }

        // TODO They claim nng_msg_get_pipe() returns -1 on error, but its return type is nng_pipe, which can't 
        //  be compared to an integer.
        nng_pipe pipe = nng_msg_get_pipe(msg);

        int pipe_id;
        if ((pipe_id = nng_pipe_id(pipe)) == -1) {
            report_nng_error("nng_pipe_id", rv);
            return 1;
        }

        struct client_state *msg_client_state;

        if (get_client_state_by_pipe_id(pipe_id) < 0) {
            // Register this pipe ID as a client.
            msg_client_state = register_client_state(pipe);
            LOG_D("Registered pipe %d as client %d", pipe_id, msg_client_state->index);
        } else {
        }

        char *buf = nng_msg_body(msg);

        LOG_I("From pipe %d: %s", pipe_id, buf);

        // Create the reply.
        nng_msg *reply;

        // Allocate the message here for convenience.
        // If insertions are made, nng will automatically reallocate the body pointer with more storage to fit the
        //  data.
        if ((rv = nng_msg_alloc(&reply, SUNNEED_MESSAGE_DEFAULT_BODY_SZ)) != 0) {
            report_nng_error("nng_msg_alloc", rv);
            return 1;
        }

        if (strncmp(SUNNEED_IPC_TEST_REQ_STR, buf, strlen(SUNNEED_IPC_TEST_REQ_STR)) == 0) {
            // Handle IPC test request.
            if ((rv = nng_msg_insert(reply, SUNNEED_IPC_TEST_REP_STR, strlen(SUNNEED_IPC_TEST_REP_STR)) != 0)) {
                report_nng_error("nng_msg_insert", rv);
                return 1;
            }

            if ((rv = nng_sendmsg(sock, reply, 0)) != 0) {
                report_nng_error("nng_sendmsg", rv);
                return 1;
            }
        } else if (strncmp(SUNNEED_IPC_REQ_GET_DEVICE_HANDLE, buf, strlen(SUNNEED_IPC_REQ_GET_DEVICE_HANDLE)) == 0) {
            // TODO They claim nng_msg_get_pipe() returns -1 on error, but its return type is nng_pipe, which can't 
            //  be compared to an integer.
            nng_pipe pipe = nng_msg_get_pipe(msg);

            int pipe_id;
            if ((pipe_id = nng_pipe_id(pipe)) == -1) {
                report_nng_error("nng_pipe_id", rv);
                return 1;
            }

            LOG_D("Pipe %d entering GET_DEVICE_HANDLE", pipe_id);

            int idx = get_client_state_by_pipe_id(pipe_id);
        }

        nng_msg_free(msg);
        nng_msg_free(reply);
    }
}
