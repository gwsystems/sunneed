#include "sunneed_listener.h"

#include "protobuf/c/server.pb-c.h"

#define SUB_RESPONSE_BUF_SZ 4096

extern struct sunneed_device devices[];
extern struct sunneed_tenant tenants[];

// Control flow:
// When a new pipe connects, we use this struct to make a mapping of its pipe ID to a tenant. Then, when further
//  requests are made, the pipe ID is used to identify a tenant to the request.
// The client will have to send some notification in order to unregister; I don't think we can tell if a pipe
//  closed.
struct tenant_pipe {
    struct sunneed_tenant *tenant;
    nng_pipe pipe;
} tenant_pipes[SUNNEED_MAX_IPC_CLIENTS];

// TODO This is probably slow -- O(n) lookup for every request made.
static struct sunneed_tenant *
tenant_of_pipe(int pipe_id) {
    for (int i = 0; i < SUNNEED_MAX_IPC_CLIENTS; i++)
        if (nng_pipe_id(tenant_pipes[i].pipe) == pipe_id)
            return tenant_pipes[i].tenant;
    return NULL;
}

// Get the PID of a pipe and use that to create a new sunneed tenant with that ID.
// TODO: This shouldn't always create a new tenant, since we want multiple processes
//  mapped to one tenant.
static struct sunneed_tenant *
register_client(nng_pipe pipe) {
    struct sunneed_tenant *tenant;

    // Get PID of pipe.
    uint64_t pid_int;
    SUNNEED_NNG_TRY(nng_pipe_get_uint64, != 0, pipe, NNG_OPT_IPC_PEER_PID, &pid_int);
    pid_t pid = (pid_t)pid_int;

    if ((tenant = sunneed_tenant_register(pid)) == NULL) {
        LOG_E("Failed to initialize tenant from PID %d", pid); 
        return NULL;
    }

    for (int i = 0; i < SUNNEED_MAX_IPC_CLIENTS; i++) {
        if (tenant_pipes[i].tenant == NULL) {
            tenant_pipes[i].tenant = tenant;
            tenant_pipes[i].pipe = pipe;
            break;
        }
    }

    return tenant;
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

// Create a mapping between this pipe and a sunneed tenant.
// TODO Currently, this just spawns a new tenant for each different pipe. We want tenants to be able to have multiple 
//  pipes to sunneed open.
static int
serve_register_client(SunneedResponse *resp, void *sub_resp_buf, nng_pipe pipe) {
    int retval;

    resp->message_type_case = SUNNEED_RESPONSE__MESSAGE_TYPE_GENERIC;

    GenericResponse *sub_resp = sub_resp_buf;
    *sub_resp = (GenericResponse)GENERIC_RESPONSE__INIT;
    resp->generic = sub_resp;

    struct sunneed_tenant *tenant = NULL;

    // Register as a new client.
    if ((tenant = register_client(pipe)) == NULL) {
        LOG_W("Registration failed for pipe %d", pipe.id);
        return 1;
    }

    LOG_D("Registered pipe %d with tenant %d", pipe.id, tenant->id);

    return 0;
}

static int
serve_unregister_client(SunneedResponse *resp, void *sub_resp_buf, nng_pipe pipe, struct sunneed_tenant *tenant) {
    int retval;

    LOG_D("Unregistering tenant %d", tenant->id);

    // Find the entry in the tenant pipe mappings.
    bool cleared = false;
    for (int i = 0; i < SUNNEED_MAX_IPC_CLIENTS; i++)
        if (tenant_pipes[i].pipe.id == pipe.id) {
            LOG_D("Removing mapping from pipe %d to tenant %d", pipe.id, tenant->id); 
            tenant_pipes[i].tenant = NULL;
            tenant_pipes[i].pipe = (nng_pipe)NNG_PIPE_INITIALIZER;
            cleared = true;
            break;
        }

    if (!cleared) {
        LOG_E("No mapping cleared when unregistering pipe %d; something is wrong with the pipe->tenant table", pipe.id);
        return 1;
    }

    if ((retval = sunneed_tenant_unregister(tenant)) != 0)
        // TODO Handle (follow the pattern of the `register_client` stuff by making a secondary `unregister` function
        //  that handles interacting with tenants).
        return 1;

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
        __attribute__((unused)) struct sunneed_tenant *tenant,
        GetDeviceHandleRequest *request) {
    struct sunneed_device *device = NULL;
    
    // Find the device with the name matching the request.
    for (unsigned int device_index = 0; device_index < MAX_DEVICES; device_index++) {
        if (!devices[device_index].is_linked)
            continue;

        if (strncmp(devices[device_index].identifier, request->name, strlen(devices[device_index].identifier)) == 0) {
            device = &devices[device_index]; 
            break;
        }
    }

    if (!device) {
        LOG_E("Unable to find requested device '%s'", request->name);
        return 1;
    }

    // Respond with the handle.
    resp->message_type_case = SUNNEED_RESPONSE__MESSAGE_TYPE_GET_DEVICE_HANDLE;
    GetDeviceHandleResponse *sub_resp = sub_resp_buf;
    *sub_resp = (GetDeviceHandleResponse)GET_DEVICE_HANDLE_RESPONSE__INIT;
    sub_resp->device_handle = device->handle;
    resp->get_device_handle = sub_resp;

    return 0;
}

static int
serve_generic_device_action(
        __attribute__((unused)) SunneedResponse *resp,
        void *sub_resp_buf,
        __attribute__((unused)) struct sunneed_tenant *tenant,
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

static int
serve_file_is_locked(
        __attribute__((unused)) SunneedResponse *resp,
        void *sub_resp_buf,
        __attribute__((unused)) struct sunneed_tenant *tenant,
        FileIsLockedRequest *request) {
    GenericResponse *sub_resp = sub_resp_buf;
    *sub_resp = (GenericResponse)GENERIC_RESPONSE__INIT;

    struct sunneed_device *locker;
    if ((locker = sunneed_device_file_is_locked(request->path)) != NULL) {
        // TODO Wait for availability, perform power calcs, etc.
    }

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
        tenant_pipes[i] = (struct tenant_pipe){.tenant = NULL,
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

        struct sunneed_tenant *tenant = NULL;

        // Find the pipe's associated tenant. If we can't find it, we error out unless the message is of type REGISTER_CLIENT.
        if ((tenant = tenant_of_pipe(pipe.id)) == NULL && request->message_type_case != SUNNEED_REQUEST__MESSAGE_TYPE_REGISTER_CLIENT) {
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
                ret = serve_register_client(&resp, sub_resp_buf, pipe);
                break;
            case SUNNEED_REQUEST__MESSAGE_TYPE_UNREGISTER_CLIENT:
                ret = serve_unregister_client(&resp, sub_resp_buf, pipe, tenant);
                break;
            case SUNNEED_REQUEST__MESSAGE_TYPE_GET_DEVICE_HANDLE:
                ret = serve_get_handle(&resp, sub_resp_buf, tenant, request->get_device_handle);
                break;
            case SUNNEED_REQUEST__MESSAGE_TYPE_DEVICE_ACTION:
                ret = serve_generic_device_action(&resp, sub_resp_buf, tenant, request->device_action);
                break;
            case SUNNEED_REQUEST__MESSAGE_TYPE_FILE_IS_LOCKED:
                ret = serve_file_is_locked(&resp, sub_resp_buf, tenant, request->file_is_locked);
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
