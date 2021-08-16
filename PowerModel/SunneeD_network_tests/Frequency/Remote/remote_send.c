#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 9999

/*
 * Send packets at various frequencies to the Pi
 * 
 * Uses UDP sockets as written, follow comments below to 
 * change to TCP sockets
 */

int
main(void)
{
        int pi_fd, read_val;
        struct sockaddr_in pi_addr;
        char msg[64];
        msg[0] = 'A';
        float timeout = 0.5;

        //change from SOCK_DGRAM to SOCK_STREAM to create TCP socket
        if((pi_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        {
                perror("failed to create socket\n");
                exit(0);
        }

        pi_addr.sin_family = AF_INET;
        pi_addr.sin_port = htons(PORT);

        if(inet_pton(AF_INET, "192.168.1.134", &pi_addr.sin_addr) <= 0)
        {
                perror("invalid address/failed to convert\n");
                exit(0);
        }

        if(connect(pi_fd, (struct sockaddr *)&pi_addr, sizeof(pi_addr)) < 0)
        {
                perror("connection failed\n");
                exit(0);
        }

        printf("connected to pi\n");
        int i;

        for(i = 0; i < 20; i++)
        {
                printf("sending: %s\n", msg);
                send(pi_fd, msg, sizeof(msg), 0);

                sleep(timeout);

                timeout += 0.5;
        }
}