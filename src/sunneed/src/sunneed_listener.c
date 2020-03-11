#include "sunneed_listener.h"

static void report_nng_error(const char *func, int rv) {
    LOG_E("nng error: (%s) %s", func, nng_strerror(rv));
}

int sunneed_listen(void) {
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

        char *buf = nng_msg_body(msg);

        LOG_I("Received message: %s", buf);

        // Create the reply.
        nng_msg *reply;

        if (strncmp(SUNNEED_IPC_TEST_REQ_STR, buf, strlen(SUNNEED_IPC_TEST_REQ_STR)) == 0) {
            if ((rv = nng_msg_alloc(&reply, strlen(SUNNEED_IPC_TEST_REP_STR))) != 0) {
                report_nng_error("nng_msg_alloc", rv);
                return 1;
            }

            if ((rv = nng_msg_insert(reply, SUNNEED_IPC_TEST_REP_STR, strlen(SUNNEED_IPC_TEST_REP_STR)) != 0)) {
                report_nng_error("nng_msg_insert", rv);
                return 1;
            }

            if ((rv = nng_sendmsg(sock, reply, 0)) != 0) {
                report_nng_error("nng_sendmsg", rv);
                return 1;
            }
        }

        nng_msg_free(msg);
        nng_msg_free(reply);
    }
}
