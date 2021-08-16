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
	int remote_fd, read_val;
//	int pi_sock;				//uncomment for TCP sockets
	struct sockaddr_in remote_addr;
	int opt = 1;
	int addrlen = sizeof(remote_addr);
	char buffer[1024] = {0};
	float timeout = 0.5;

	//change from SOCK_DGRAM to SOCK_STREAM for TCP sockets
	if((remote_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0)
	{
		perror("socket failed\n");
		exit(0);
	}

	if(setsockopt(remote_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
	{
		perror("setsockopt failed\n");
		exit(0);
	}

	remote_addr.sin_family = AF_INET;
	remote_addr.sin_addr.s_addr = INADDR_ANY;
	remote_addr.sin_port = htons(PORT);

	if(bind(remote_fd, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) < 0)
	{
		perror("bind failed\n");
		exit(0);
	}

	/*
	 * Uncomment this section if using TCP sockets, comment out for UDP sockets
	 */

	/*
	if(listen(remote_fd, 3) < 0)
	{
		perror("listen failed\n");
		exit(0);
	}

	if((pi_sock = accept(pi_fd, (struct sockaddr *)&remote_addr, (socklen_t*)&addrlen)) < 0)
	{
		perror("accept failed\n");
		exit(0);
	}
	*/

	printf("connected to remote host\n");
	int i;

	while((read_val = read(remote_fd, buffer, 1024)) > 0)
	{
		printf("received: %s\n", buffer);
		sleep(timeout);

		timeout += 0.5;
	}
}
