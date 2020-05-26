#include "sunneed_listener.h"

#include "protobuf/c/server.pb-c.h"

#define SUB_RESPONSE_BUF_SZ 4096

extern struct sunneed_device devices[];
extern struct sunneed_tenant tenants[];

// Control flow:
// When a new client (identified by pipe ID) connects, we register it in the clint_states array.
// Once registered, clients can send requests over NNG. Requests consist of a token word that describes the request
//  being made, optionally followed by a newline character delimiting the arguments for that command.
// The client will have to send some notification in order to unregister; I don't think we can tell if a pipe
//  closed.
struct client_state {
    int index;
    sunneed_tenant_id_t tenant_id;
    bool is_active;
    nng_pipe pipe;
};

struct client_state client_states[SUNNEED_MAX_IPC_CLIENTS];

// TODO This is probably slow -- O(n) lookup for every request made.
static struct client_state *
get_client_state_by_pipe_id(int pipe_id) {
    for (int i = 0; i < SUNNEED_MAX_IPC_CLIENTS; i++)
        if (nng_pipe_id(client_states[i].pipe) == pipe_id)
            return &client_states[i];
    return NULL;
}

static struct client_state *
register_client_state(nng_pipe pipe) {
    int idx;
    for (idx = 0; idx < SUNNEED_MAX_IPC_CLIENTS; idx++)
        if (!client_states[idx].is_active)
            break;
    if (idx == SUNNEED_MAX_IPC_CLIENTS)
        return NULL;

    if (!tenants[idx].is_active) {
        // Register tenant.
        uint64_t pid_int;

        SUNNEED_NNG_TRY(nng_pipe_get_uint64, !=0, pipe, NNG_OPT_IPC_PEER_PID, &pid_int);

        pid_t pid = (pid_t)pid_int;

        if (sunneed_tenant_register(idx, pid) != 0) {
            LOG_E("Failed to register tenant %d", idx);
            return NULL;
        }
    }

    // TODO Get a real tenant ID somehow.
    client_states[idx] = (struct client_state){.index = idx, .tenant_id = idx, .is_active = true, .pipe = pipe};

    return &client_states[idx];
}


// The `serve_*` methods take a `sub_resp_buf` parameter. This is a pointer to a buffer in which the client
//  can store their sub-response (the message in the oneof field of the SunneedResponse). Example:
//
//    GetDeviceHandleResponse *sub_resp = sub_resp_buf;
//    *sub_resp = (GetDeviceHandleResponse)GET_DEVICE_HANDLE_RESPONSE__INIT;
//
// This example writes the initializer for the `GetDeviceHandleResponse` to the address pointed to by
//  `sub_resp_buf`.
// The rationale for this whole process comes next: once the `serve_*` function returns, its sub-response
//  data is contained within the buffer, to which a pointer is in scope in the main request listening
//  loop.

static int
serve_register_client(SunneedResponse *resp, void *sub_resp_buf, nng_pipe pipe, struct client_state **state) {
    resp->message_type_case = SUNNEED_RESPONSE__MESSAGE_TYPE_GENERIC;

    GenericResponse *sub_resp = sub_resp_buf;
    *sub_resp = (GenericResponse)GENERIC_RESPONSE__INIT;
    resp->generic = sub_resp;

    // If the caller doesn't specify `state` we set it to this helper variable.
    struct client_state *s;
    if (state == NULL) {
        state = &s;
    }

    if ((*state = get_client_state_by_pipe_id(pipe.id)) != NULL) {
        LOG_W("Registration request for already-registered pipe %d", pipe.id);
        return 1;
    }

    // Register as a new client.
    if ((*state = register_client_state(pipe)) == NULL) {
        LOG_W("Registration failed for pipe %d", pipe.id);
        return 1;
    }

    LOG_D("Registered pipe %d as client %d", pipe.id, (*state)->index);

    return 0;
}

static int
serve_unregister_client(
        SunneedResponse *resp,
        void *sub_resp_buf,
        struct client_state *client_state) {
    LOG_D("Unregistering client %d", client_state->index);

    // Deactivate records.
    client_state->is_active = false;
    tenants[client_state->tenant_id].is_active = false;

    resp->message_type_case = SUNNEED_RESPONSE__MESSAGE_TYPE_GENERIC;
    GenericResponse *sub_resp = sub_resp_buf;
    *sub_resp = (GenericResponse)GENERIC_RESPONSE__INIT;
    resp->generic = sub_resp;

    return 0;
}

static int
serve_get_handle(
        SunneedResponse *resp,
        void *sub_resp_buf,
        __attribute__((unused)) struct client_state *client_state,
        GetDeviceHandleRequest *request) {
    static int handle_cur = 0;

    char filename[SUNNEED_DEVICE_PATH_MAX_LEN];

    int len;
    if ((len = snprintf(filename, SUNNEED_DEVICE_PATH_MAX_LEN, "build/device/%s.so", request->name))
               > SUNNEED_DEVICE_PATH_MAX_LEN) {
        LOG_E("sunneed error: device name '%s' is too long", request->name);
        return 1;
    }

    void *dlhandle = dlopen(filename, RTLD_LAZY | RTLD_LOCAL);
    if (!dlhandle) {
        LOG_E("Error loading device from '%s': %s", filename, dlerror());
        return 1;
    }

    size_t name_sz = strlen(request->name);

    devices[handle_cur] = (struct sunneed_device){.dlhandle = dlhandle,
                                                  .handle = handle_cur,
                                                  .identifier = malloc(name_sz),
                                                  .get = dlsym(dlhandle, "get"),
                                                  .power_consumption = dlsym(dlhandle, "power_consumption"),
                                                  .is_linked = false};
    strncpy(devices[handle_cur].identifier, request->name, name_sz);

    if (devices[handle_cur].get == NULL || devices[handle_cur].power_consumption == NULL) {
        LOG_E("Error linking device '%s': %s", request->name, dlerror());
        return 1;
    }

    devices[handle_cur].is_linked = true;

    resp->message_type_case = SUNNEED_RESPONSE__MESSAGE_TYPE_GET_DEVICE_HANDLE;
    GetDeviceHandleResponse *sub_resp = sub_resp_buf;
    *sub_resp = (GetDeviceHandleResponse)GET_DEVICE_HANDLE_RESPONSE__INIT;
    sub_resp->device_handle = handle_cur;
    resp->get_device_handle = sub_resp;

    LOG_I("Linked device '%s' at handle %d", request->name, handle_cur);

    return 0;
}

static int
serve_generic_device_action(
        __attribute__((unused)) SunneedResponse *resp,
        void *sub_resp_buf,
        __attribute__((unused)) struct client_state *client_state,
        GenericDeviceActionRequest *request) {
    // TODO SAFETY (DON'T JUST ACCEPT ANY HANDLE)!!!!!
    if (!devices[request->device_handle].is_linked) {
        LOG_W("Action request sent for device %d, which is not linked", request->device_handle);
        return 1;
    }

    LOG_I("Calling '%s' function 'get'", devices[request->device_handle].identifier);
    devices[request->device_handle].get(request->data.data);

    GenericResponse *sub_resp = sub_resp_buf;
    *sub_resp = (GenericResponse)GENERIC_RESPONSE__INIT;

    return 0;
}

static void
report_nng_error(const char *func, int rv) {
    LOG_E("nng error: (%s) %s", func, nng_strerror(rv));
}

int
sunneed_listen(void) {
    SUNNEED_NNG_SET_ERROR_REPORT_FUNC(report_nng_error);

    // Initialize client states.
    for (int i = 0; i < MAX_TENANTS; i++) {
        client_states[i] = (struct client_state){.is_active = false,
                                                 // TODO Why do I need to cast this...
                                                 .pipe = (nng_pipe)NNG_PIPE_INITIALIZER};
    }

    nng_socket sock;

    LOG_I("Starting listener loop...");

    // Make a socket and attach it to the sunneed URL.
    SUNNEED_NNG_TRY_RET(nng_rep0_open, != 0, &sock);
    SUNNEED_NNG_TRY_RET(nng_listen, < 0, sock, SUNNEED_LISTENER_URL, NULL, 0);

    // Buffer for `serve_` methods to write their sub-response to.
    void *sub_resp_buf = malloc(SUB_RESPONSE_BUF_SZ);
    // TODO Check malloc.

    // Await messages.
    for (;;) {
        nng_msg *msg;

        SUNNEED_NNG_TRY_RET(nng_recvmsg, != 0, sock, &msg, NNG_FLAG_ALLOC);

        // TODO They claim nng_msg_get_pipe() returns -1 on error, but its return type is nng_pipe, which can't
        //  be compared to an integer.
        nng_pipe pipe = nng_msg_get_pipe(msg);

        // Get contents of message.
        SunneedRequest *request = sunneed_request__unpack(NULL, nng_msg_len(msg), nng_msg_body(msg));

        struct client_state *msg_client_state = NULL;

        // Find the pipe's associated client state. If we can't find it, we error out unless the message is of type
        //  REGISTER_CLIENT..
        if (!msg_client_state && (msg_client_state = get_client_state_by_pipe_id(pipe.id)) == NULL
            && request->message_type_case != SUNNEED_REQUEST__MESSAGE_TYPE_REGISTER_CLIENT) {
            // This client has not registered!
            LOG_W("Received message from %d, who is not registered.", pipe.id);
            goto end;
        }

        // Begin setting up our response.
        SunneedResponse resp = SUNNEED_RESPONSE__INIT;
        int ret = -1;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
        switch (request->message_type_case) {
            case SUNNEED_REQUEST__MESSAGE_TYPE__NOT_SET:
                LOG_W("Request from pipe %d has no message type set.", pipe.id);
                ret = -1;
                break;
            case SUNNEED_REQUEST__MESSAGE_TYPE_REGISTER_CLIENT:
                ret = serve_register_client(&resp, sub_resp_buf, pipe, &msg_client_state);
                break;
            case SUNNEED_REQUEST__MESSAGE_TYPE_UNREGISTER_CLIENT:
                ret = serve_unregister_client(&resp, sub_resp_buf, msg_client_state);
                break;
            case SUNNEED_REQUEST__MESSAGE_TYPE_GET_DEVICE_HANDLE:
                ret = serve_get_handle(&resp, sub_resp_buf, msg_client_state, request->get_device_handle);
                break;
            case SUNNEED_REQUEST__MESSAGE_TYPE_DEVICE_ACTION:
                ret = serve_generic_device_action(&resp, sub_resp_buf, msg_client_state, request->device_action);
                break;
        }
#pragma GCC diagnostic pop

        resp.status = ret;

        // Create and send the response message.
        nng_msg *resp_msg;
        int resp_len = sunneed_response__get_packed_size(&resp);
        void *resp_buf = malloc(resp_len);
        sunneed_response__pack(&resp, resp_buf);

        SUNNEED_NNG_TRY(nng_msg_alloc, != 0, &resp_msg, resp_len);
        SUNNEED_NNG_TRY(nng_msg_insert, != 0, resp_msg, resp_buf, resp_len);
        SUNNEED_NNG_TRY(nng_sendmsg, != 0, sock, resp_msg, 0);

    end:
        sunneed_request__free_unpacked(request, NULL);
        nng_msg_free(resp_msg);
        nng_msg_free(msg);
    }

    free(sub_resp_buf);
}
