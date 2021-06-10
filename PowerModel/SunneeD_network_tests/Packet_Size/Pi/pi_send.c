#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 9999

/*
 * send varying packet sizes from the Pi to a remote host using UDP sockets
 * see comments below for how to convert this code to use TCP sockets
 */

int
main(void)
{
	int remote_fd, read_val;
	struct sockaddr_in remote_addr;
	char msg[64];
	char buffer[1024] = {0};

	msg[0] = 'A';

	//create UDP socket, change SOCK_DGRAM to SOCK_STREAM for a TCP socket
	if((remote_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("failed to create socket\n");
		return 0;
	}

	remote_addr.sin_family = AF_INET;
	remote_addr.sin_port = htons(PORT);

	if(inet_pton(AF_INET, "192.168.1.214", &remote_addr.sin_addr) <= 0)
	{
		perror("invalid address/failed to convert address\n");
		return 0;
	}

	if(connect(remote_fd, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) < 0)
	{
		perror("failed to connect\n");
		return 0;
	}

	printf("connected to remote host\n");
	int i;

	for(i = 0; i < 64; i++)
	{
		printf("msg to send: %s\n", msg);
		send(remote_fd, msg, strlen(msg), 0);

		msg[i + 1] = (char)(msg[0] + i + 1);

		sleep(1);
	}
}
