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
	int sock_fd, remote_sock, read_val;
	struct sockaddr_in remote_addr;
	int opt = 1;
	int addrlen = sizeof(remote_addr);
	char buffer[1024] = {0};


	//create UDP socket, change from SOCK_DGRAM to SOCK_STREAM for a TCP socket
	if((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0)
	{
		perror("socket failure\n");
		return 0;
	}

	if(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
	{
		perror("setsockopt failed\n");
		return 0;
	}

	remote_addr.sin_family = AF_INET;
	remote_addr.sin_addr.s_addr = INADDR_ANY;
	remote_addr.sin_port = htons(PORT);

	if(bind(sock_fd, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) < 0)
	{
		perror("bind failed\n");
		return 0;
	}
	/*
	 * Uncomment this section if testing with TCP (make sure to change the socket from
	 * SOCK_DGRAM to SOCK_STREAM
	if(listen(sock_fd, 3) < 0)
	{
		perror("listen failed\n");
		return 0;
	}

	if((remote_sock = accept(sock_fd, (struct sockaddr *)&remote_addr, (socklen_t*)&addrlen)) < 0)
	{
		perror("accept failed\n");
		return 0;
	}*/
	printf("connected to remote host\n");

	while((read_val = read(sock_fd, buffer, 1024)) != 0)
	{
		printf("received %s\n", buffer);
		sleep(1);
	}
}
