#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 9999

/*
 * Send varying sized packets from this host to the Pi
 * uses UDP sockets to keep the packet size consistent with what is being sent
 * 
 * to test with TCP, change SOCK_DGRAM to SOCK_STREAM and see the note for testing with
 * TCP in recv_test_size.c on the Pi
 */
int
main(void)
{
        int pi_fd, read_val;
        struct sockaddr_in pi_addr;
        char msg[64];
        msg[0] = 'A';

        

        char buffer[1024] = {0};

        if((pi_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        {
                fprintf(stderr, "failed to create socket\n");
                return 0;
        }

        pi_addr.sin_family = AF_INET;
        pi_addr.sin_port = htons(PORT);

        if(inet_pton(AF_INET, "192.168.1.134", &pi_addr.sin_addr)<=0)
        {
                fprintf(stderr, "invalid address/failed to convert address\n");
                return 0;
        }

        if(connect(pi_fd, (struct sockaddr *)&pi_addr, sizeof(pi_addr)) < 0)
        {
                fprintf(stderr, "connection failed\n");
                return 0;
        }

        printf("connected to pi\n");
        int i;
        /*
         * starting with an empty string, add one char at a time up to 64
         * since chars are 1 byte each, this should be roughly equivalent to
         * increasing the packet size by one byte for each send
         */
        for(i = 0; i < 64; i++)
        {
                printf("msg to send: %s\n", msg);
                send(pi_fd, msg, sizeof(msg), 0);

                sleep(1);

                msg[i + 1] = (char)(msg[0] + 1 + i);
        }
}