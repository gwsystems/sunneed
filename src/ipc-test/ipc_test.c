#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/reqrep0/req.h>

#define SUNNEED_LISTENER_URL "ipc:///tmp/sunneed.ipc"

static void fatal(const char *func, int rv) {
    fprintf(stderr, "%s: %s\n", func, nng_strerror(rv));
    exit(1);
}

int main(int argc, char const* argv[]) {
    nng_socket sock;
    int rv;
    size_t sz;
    char *buf = NULL;

    if ((rv = nng_req0_open(&sock)) != 0) {
        fatal("nng_socket", rv);
    }
    if ((rv = nng_dial(sock, SUNNEED_LISTENER_URL, NULL, 0)) != 0) {
        fatal("nng_dial", rv);
    }

    printf("Sending request.\n");


    if ((rv = nng_send(sock, "REQ", strlen("REQ") + 1, 0)) != 0) {
        fatal("nng_send", rv);
    }
    if ((rv = nng_recv(sock, &buf, &sz, NNG_FLAG_ALLOC)) != 0) {
        fatal("nng_recv", rv);
    }

    printf("Received reply: %s\n", buf);
    nng_free(buf, sz);
    nng_close(sock);

    return 0;
}
