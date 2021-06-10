#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 9999

int
main(void)
{
        int pi_fd, read_val;
//      int pi_sock;                                    //uncomment this if using TCP sockets
        struct sockaddr_in pi_addr;
        int opt = 1;
        int addrlen = sizeof(pi_addr);
        char buffer[1024] = {0};
        float downtime = 0.5;

        //create UDP socket, change SOCK_DGRAM to SOCK_STREAM for a TCP socket
        if((pi_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        {
                perror("failed to create socket\n");
                return 0;
        }

        if(setsockopt(pi_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
        {
                perror("setsockopt failed\n");
                return 0;
        }

        pi_addr.sin_family = AF_INET;
        pi_addr.sin_addr.s_addr = INADDR_ANY;
        pi_addr.sin_port = htons(PORT);

        if(bind(pI_fd, (struct sockaddr *)&pi_addr, sizeof(pi_addr)) < 0)
        {
                perror("bind failed\n");
                return 0;
        }

        /*
         * Uncomment this section is using TCP sockets
         */

        /*
        if(listen(pi_fd, 3) < 0)
        {
                perror("listen failed\n");
                return 0;
        }

        if((pi_sock = accept(pi_fd, (struct sockaddr *)&pi_addr, (socklen_t*)&addrlen)) < 0)
        {
                perror("accept failed\n");
                return 0;
        }
        */

       printf("connected to pi\n");
       while((read_val = read(pi_fd, buffer, 1024)) > 0)
       {
               printf("received: %s\n", buffer);
               sleep(downtime);

                downtime += 0.5;
       }
}