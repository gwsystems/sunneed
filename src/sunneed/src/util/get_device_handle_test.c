#include "../protobuf/c/server.pb-c.h"
#include "../shared/sunneed_ipc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/reqrep0/req.h>

#define DEVICE_NAME "device"

#define MSG_SIZE 64

static void
fatal(const char *func, int rv) {
    fprintf(stderr, "%s: %s\n", func, nng_strerror(rv));
    exit(1);
}

int
main(void) {
    SUNNEED_NNG_SET_ERROR_REPORT_FUNC(fatal);

    nng_socket sock;

    SUNNEED_NNG_TRY(nng_req0_open, != 0, &sock);
    SUNNEED_NNG_TRY(nng_dial, != 0, sock, SUNNEED_LISTENER_URL, NULL, 0);

    printf("Sending request.\n");

    nng_msg *msg;

    SunneedRequest req = SUNNEED_REQUEST__INIT;
    RegisterClientRequest register_req = REGISTER_CLIENT_REQUEST__INIT;
    req.message_type_case = SUNNEED_REQUEST__MESSAGE_TYPE_REGISTER_CLIENT;
    req.register_client = &register_req;

    int req_len = sunneed_request__get_packed_size(&req);
    void *buf = malloc(req_len);
    sunneed_request__pack(&req, buf);

    SUNNEED_NNG_TRY(nng_msg_alloc, != 0, &msg, req_len);
    SUNNEED_NNG_TRY(nng_msg_insert, != 0, msg, buf, req_len);
    SUNNEED_NNG_TRY(nng_sendmsg, != 0, sock, msg, 0);

    nng_msg *reply;

    SUNNEED_NNG_TRY(nng_recvmsg, != 0, sock, &reply, 0);

    SunneedResponse *resp = sunneed_response__unpack(NULL, nng_msg_len(reply), nng_msg_body(reply));

    if (resp->status != 0) {
        printf("FAILED: registration failed\n");
        return 1;
    }

    printf("Registered successfully.\n");

    nng_msg_free(msg);
    nng_msg_free(reply);
    free(buf);
    sunneed_response__free_unpacked(resp, NULL);

    // Make the request for a device handle.
    req = (SunneedRequest)SUNNEED_REQUEST__INIT;
    GetDeviceHandleRequest handle_req = GET_DEVICE_HANDLE_REQUEST__INIT;
    handle_req.name = "device";
    req.message_type_case = SUNNEED_REQUEST__MESSAGE_TYPE_GET_DEVICE_HANDLE;
    req.get_device_handle = &handle_req;

    req_len = sunneed_request__get_packed_size(&req);
    buf = malloc(req_len);
    sunneed_request__pack(&req, buf);

    SUNNEED_NNG_TRY(nng_msg_alloc, != 0, &msg, req_len);
    SUNNEED_NNG_TRY(nng_msg_insert, != 0, msg, buf, req_len);
    SUNNEED_NNG_TRY(nng_sendmsg, != 0, sock, msg, 0);

    SUNNEED_NNG_TRY(nng_recvmsg, != 0, sock, &reply, 0);
    resp = sunneed_response__unpack(NULL, nng_msg_len(reply), nng_msg_body(reply));

    if (resp->status != 0) {
        printf("FAILED: error getting response (%d)\n", resp->status);
        return 1;
    } else if (resp->message_type_case != SUNNEED_RESPONSE__MESSAGE_TYPE_GET_DEVICE_HANDLE) {
        printf("FAILED: response message type was incorrect (%d)\n", resp->message_type_case);
        return 1;
    }

    printf("Got device handle: %d\n", resp->get_device_handle->device_handle);
    int handle = resp->get_device_handle->device_handle;

    nng_msg_free(msg);
    nng_msg_free(reply);
    free(buf);
    sunneed_response__free_unpacked(resp, NULL);

    // Make the generic device action request.
    req = (SunneedRequest)SUNNEED_REQUEST__INIT;
    GenericDeviceActionRequest action_req = GENERIC_DEVICE_ACTION_REQUEST__INIT;
    action_req.device_handle = handle;
    req.message_type_case = SUNNEED_REQUEST__MESSAGE_TYPE_DEVICE_ACTION;
    req.device_action = &action_req;

    req_len = sunneed_request__get_packed_size(&req);
    buf = malloc(req_len);
    sunneed_request__pack(&req, buf);

    SUNNEED_NNG_TRY(nng_msg_alloc, != 0, &msg, req_len);
    SUNNEED_NNG_TRY(nng_msg_insert, != 0, msg, buf, req_len);
    SUNNEED_NNG_TRY(nng_sendmsg, != 0, sock, msg, 0);

    nng_msg_free(msg);
    nng_msg_free(reply);
    free(buf);

    // Make the generic device action request.
    req = (SunneedRequest)SUNNEED_REQUEST__INIT;
    UnregisterClientRequest unreg_req = UNREGISTER_CLIENT_REQUEST__INIT;
    req.message_type_case = SUNNEED_REQUEST__MESSAGE_TYPE_UNREGISTER_CLIENT;
    req.unregister_client = &unreg_req;

    req_len = sunneed_request__get_packed_size(&req);
    buf = malloc(req_len);
    sunneed_request__pack(&req, buf);

    SUNNEED_NNG_TRY(nng_msg_alloc, != 0, &msg, req_len);
    SUNNEED_NNG_TRY(nng_msg_insert, != 0, msg, buf, req_len);
    SUNNEED_NNG_TRY(nng_sendmsg, != 0, sock, msg, 0);

    nng_msg_free(msg);
    nng_msg_free(reply);
    free(buf);

    nng_close(sock);

    return 0;
}
