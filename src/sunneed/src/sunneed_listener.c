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
        char *buf = NULL;
        size_t sz;

        if ((rv = nng_recv(sock, &buf, &sz, NNG_FLAG_ALLOC)) != 0) {
            report_nng_error("nng_recv", rv);
            return 1;
        }

        LOG_I("Received message: %s", buf);

        if (strcmp(SUNNEED_IPC_TEST_REQ_STR, buf) == 0) {
            if ((rv = nng_send(sock, SUNNEED_IPC_TEST_REP_STR, strlen(SUNNEED_IPC_TEST_REP_STR) + 1, 0)) != 0) {
                report_nng_error("nng_send", rv);
                return 1;
            }
        }

        nng_free(buf, sz);
    }
}
