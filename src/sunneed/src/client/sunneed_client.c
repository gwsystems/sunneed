#include "sunneed_client.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

nng_socket sunneed_socket;

static void
nngfatal(const char *func, int rv) {
    fprintf(stderr, "%s: %s\n", func, nng_strerror(rv));
    exit(1);
}

static SunneedResponse *
receive_and_unpack(SunneedResponse__MessageTypeCase message_type) {
    nng_msg *reply;
    SUNNEED_NNG_TRY(nng_recvmsg, != 0, sunneed_socket, &reply, 0);

    SunneedResponse *resp = sunneed_response__unpack(NULL, nng_msg_len(reply), nng_msg_body(reply));

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

    nng_msg *msg;

    SunneedRequest req = SUNNEED_REQUEST__INIT;
    req.message_type_case = SUNNEED_REQUEST__MESSAGE_TYPE_REGISTER_CLIENT;
    RegisterClientRequest register_req = REGISTER_CLIENT_REQUEST__INIT;
    register_req.name = malloc(strlen(name));
    if (!register_req.name) {
        fprintf(stderr, "failed to allocate memory for client name\n");
        return -1;
    }
    strcpy(register_req.name, name);
    req.register_client = &register_req;

    int req_len = sunneed_request__get_packed_size(&req);
    void *buf = malloc(req_len);
    sunneed_request__pack(&req, buf);

    SUNNEED_NNG_TRY(nng_msg_alloc, != 0, &msg, req_len);
    SUNNEED_NNG_TRY(nng_msg_insert, != 0, msg, buf, req_len);
    SUNNEED_NNG_TRY(nng_sendmsg, != 0, sunneed_socket, msg, 0);

    nng_msg *reply;

    SUNNEED_NNG_TRY(nng_recvmsg, != 0, sunneed_socket, &reply, 0);

    SunneedResponse *resp = sunneed_response__unpack(NULL, nng_msg_len(reply), nng_msg_body(reply));

    if (resp->status != 0) {
        return resp->status;
    }

    nng_msg_free(msg);
    nng_msg_free(reply);
    free(register_req.name);
    free(buf);
    sunneed_response__free_unpacked(resp, NULL);

    return 0;
}

int
sunneed_client_get_device_handle(const char *name, sunneed_device_handle_t *handle) {
    // TODO Check socket opened.

    SunneedRequest req = SUNNEED_REQUEST__INIT;
    req.message_type_case = SUNNEED_REQUEST__MESSAGE_TYPE_GET_DEVICE_HANDLE;
    GetDeviceHandleRequest handle_req = GET_DEVICE_HANDLE_REQUEST__INIT;
    handle_req.name = malloc(strlen(name));
    if (!handle_req.name) {
        FATAL(-1, "failed to allocate memory for client name");
    }
    strcpy(handle_req.name, name);
    req.get_device_handle = &handle_req;

    PACK_AND_SEND(req);
    free(handle_req.name);

    nng_msg *reply;

    SUNNEED_NNG_TRY(nng_recvmsg, != 0, sunneed_socket, &reply, 0);

    SunneedResponse *resp = sunneed_response__unpack(NULL, nng_msg_len(reply), nng_msg_body(reply));

    if (resp->status != 0) {
        return resp->status;
    } else if (resp->message_type_case != SUNNEED_RESPONSE__MESSAGE_TYPE_GET_DEVICE_HANDLE) {
        FATAL(-1, "incorrect message type received");
    }

    GetDeviceHandleResponse *handle_resp = resp->get_device_handle;
    printf("Handle is %d\n", handle_resp->device_handle);
    *handle = handle_resp->device_handle;

    nng_msg_free(reply);
    sunneed_response__free_unpacked(resp, NULL);

    return 0;
}

int
sunneed_client_check_locked_file(const char *pathname) {
    // TODO Check socket opened.

    SunneedRequest req = SUNNEED_REQUEST__INIT;
    req.message_type_case = SUNNEED_REQUEST__MESSAGE_TYPE_FILE_IS_LOCKED;
    FileIsLockedRequest file_lock_req = FILE_IS_LOCKED_REQUEST__INIT;
    file_lock_req.path = malloc(strlen(pathname));
    if (!file_lock_req.path) {
        FATAL(-1, "failed to allocated memory for path");
    }
    strncpy(file_lock_req.path, pathname, strlen(pathname));
    req.file_is_locked = &file_lock_req;

    PACK_AND_SEND(req);
    free(file_lock_req.path);
}

int
sunneed_client_disconnect(void) {
    // TODO Check socket opened.

    SunneedRequest req = SUNNEED_REQUEST__INIT;
    req.message_type_case = SUNNEED_REQUEST__MESSAGE_TYPE_UNREGISTER_CLIENT;
    UnregisterClientRequest unregister_req = UNREGISTER_CLIENT_REQUEST__INIT;
    req.unregister_client = &unregister_req;

    PACK_AND_SEND(req);

    SunneedResponse *resp = receive_and_unpack(SUNNEED_RESPONSE__MESSAGE_TYPE_GENERIC);
    if (resp == NULL) {
        // TODO Handle
    }

    printf("Unregistered.\n");
    return 0;
}
