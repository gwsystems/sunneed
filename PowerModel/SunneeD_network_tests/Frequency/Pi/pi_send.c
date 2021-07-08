#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 9999

/*
 * Send packets at various frequencies, allowing the network card to idle for varying times
 * starts at 500 ms and increases by 500 ms every iteration
 *
 * Uses UDP sockets as written, follow the notes in comments below to change to TCP sockets
 */

int
main(void)
{
	int remote_fd, read_val;
	struct sockaddr_in remote_addr;
	char msg[64] = {'A'};
	float downtime = 0.5;

	//create UDP socket, change SOCK_DGRAM to SOCK_STREAM to create TCP sockets
	if((remote_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket failed\n");
		exit(0);
	}

	remote_addr.sin_family = AF_INET;
	remote_addr.sin_port = htons(PORT);

	if(inet_pton(AF_INET, "192.168.1.214", &remote_addr.sin_addr) <= 0)
	{
		perror("invalid address/failed to convert\n");
		exit(0);
	}

	if(connect(remote_fd, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) < 0)
	{
		perror("failed to connect\n");
		exit(0);
	}

	printf("connected to remote host\n");

	int i;
	for (i = 0; i < 20; i++)
	{
		printf("sending msg\n");
		send(remote_fd, msg, sizeof(msg), 0);

		sleep(downtime);

		downtime += 0.5;
	}
}
