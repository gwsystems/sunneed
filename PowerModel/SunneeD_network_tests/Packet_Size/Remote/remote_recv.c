#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 9999

int
main(void)
{
        int pi_fd, read_val;
//      int pi_sock;                            //uncomment this for TCP sockets
        struct sockaddr_in pi_addr;
        int opt = 1;
        int addrlen = sizeof(pi_addr);
        char buffer[1024] = {0};

        //create UDP socket, change from SOCK_DGRAM to SOCK_STREAM for TCP sockets and uncomment the section below
        if((pi_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0)
        {
                perror("socket failed\n");
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

        if(bind(pi_fd, (struct sockaddr *)&pi_addr, sizeof(pi_addr)) < 0)
        {
                perror("bind failed\n");
                return 0;
        }

        /*
         * Uncomment this section if using TCP sockets, comment out if using UDP sockets
         */

//      if(listen(pi_fd, 3) < 0)
//      {
//              perror("listen failed\n");
//              return 0;
//      }

//      if((pi_sock = accept(pi_fd, (struct sockaddr *)&remote_addr, (socklen_t*)&addrlen)) < 0)
//      {
//              perror("accept failed\n");
//              return 0;
//      }

        printf("connected to pi\n");
        while((read_val = read(pi_fd, buffer, 1024)) > 0)
        {
                printf("received %s\n", buffer);
                sleep(1);
        }
}