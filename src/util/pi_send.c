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
	char *packets[8];

	char packet_size_1 = 'A';
	packets[0] = &packet_size_1;

	char packet_size_4[4];
	memset(packet_size_4, 'a', 4);
	packets[1] = packet_size_4;

	char packet_size_8[8];
	memset(packet_size_8, 'B', 8);
	packets[2] = packet_size_8;

	char packet_size_16[16];
	memset(packet_size_16, 'b', 16);
	packets[3] = packet_size_16;

	char packet_size_32[32];
	memset(packet_size_32, 'C', 32);
	packets[4] = packet_size_32;

	char packet_size_64[64];
	memset(packet_size_64, 'c', 64);
	packets[5] = packet_size_64;

	char packet_size_128[128];
	memset(packet_size_128, 'D', 128);
	packets[6] = packet_size_128;

	char packet_size_256[256];
	memset(packet_size_256, 'd', 256);
	packets[7] = packet_size_256;
	
	int index;
	int packet_sizes[8] = {1, 4, 8, 16, 32, 64, 128, 256};
	

	//create UDP socket, change SOCK_DGRAM to SOCK_STREAM to create TCP sockets
	

	//while(1);
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


	int i, j;
	for (i = 0; i < 2000; i++)
	{
		index = rand() % 8;
		//msg = (char *) malloc(packet_sizes[index] * sizeof(char));
		/*for(j = 0; j < packet_sizes[index]-1; j++)
		{
			msg[j] = 'a';
		}*/
		//msg[packet_sizes[index] - 1] = '\0';
		printf("msg size: %d\n", packet_sizes[index]);
		send(remote_fd, packets[index], packet_sizes[index], 0);

		//free(msg);
		sleep(0.5);
	}

}
