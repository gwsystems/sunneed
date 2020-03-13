#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/reqrep0/req.h>

#include "../sunneed_listener.h"

static void fatal(const char *func, int rv) {
    fprintf(stderr, "%s: %s\n", func, nng_strerror(rv));
    exit(1);
}

int main(int argc, char const* argv[]) {
    nng_socket sock;
    int rv;

    if ((rv = nng_req0_open(&sock)) != 0) {
        fatal("nng_socket", rv);
    }
    if ((rv = nng_dial(sock, SUNNEED_LISTENER_URL, NULL, 0)) != 0) {
        fatal("nng_dial", rv);
    }

    printf("Sending request.\n");

    nng_msg *msg;
    if ((rv = nng_msg_alloc(&msg, strlen(SUNNEED_IPC_TEST_REQ_STR))) != 0) {
        fatal("nng_msg_alloc", rv);
        return 1;
    }

    if ((rv = nng_msg_insert(msg, SUNNEED_IPC_REQ_GET_DEVICE_HANDLE, strlen(SUNNEED_IPC_REQ_GET_DEVICE_HANDLE))) != 0) {
        fatal("nng_msg_insert", rv);
        return 1;
    }

    if ((rv = nng_sendmsg(sock, msg, 0)) != 0) {
        fatal("nng_sendmsg", rv);
    }

    nng_msg *reply;

    if ((rv = nng_recvmsg(sock, &reply, 0)) != 0) {
        fatal("nng_recvmsg", rv);
    }

    char *buf = nng_msg_body(reply);

    printf("Received reply: %s\n", buf);

    nng_msg_free(msg);
    nng_msg_free(reply);
    nng_close(sock);

    return 0;
}
