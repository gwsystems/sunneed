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
    SUNNEED_NNG_SET_ERROR_REPORT_FUNC(fatal);

    nng_socket sock;

    SUNNEED_NNG_TRY(nng_req0_open, !=0, &sock);
    SUNNEED_NNG_TRY(nng_dial, !=0, sock, SUNNEED_LISTENER_URL, NULL, 0);

    printf("Sending request.\n");

    nng_msg *msg;
    SUNNEED_NNG_TRY(nng_msg_alloc, !=0, &msg, strlen(SUNNEED_IPC_TEST_REQ_STR));
    SUNNEED_NNG_TRY(nng_msg_insert, !=0, msg, SUNNEED_IPC_TEST_REQ_STR, strlen(SUNNEED_IPC_TEST_REQ_STR)); 

    SUNNEED_NNG_TRY(nng_sendmsg, !=0, sock, msg, 0);

    nng_msg *reply;
    SUNNEED_NNG_TRY(nng_recvmsg, !=0, sock, &reply, 0);

    char *buf = nng_msg_body(reply);
    printf("Received reply: %s\n", buf);

    if (strcmp(buf, SUNNEED_IPC_TEST_REP_STR) != 0) {
        printf("FAILED: reply was invalid (expected \"REP\"");
    }
    
    nng_msg_free(msg);
    nng_msg_free(reply);
    nng_close(sock);

    return 0;
}
