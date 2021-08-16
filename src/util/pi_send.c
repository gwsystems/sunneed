#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

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
	
	int size_index, time_index;
	int packet_sizes[8] = {1, 4, 8, 16, 32, 64, 128, 256};
	double delay_times[14] = {0.25, 0.5, 0.75, 1.0, 1.5, 2.0, 2.5, 5.0, 7.5, 10.0, 12.5, 15.0, 17.5, 20.0};
	

	//create UDP socket, change SOCK_DGRAM to SOCK_STREAM to create TCP sockets
	

	//while(1);
	if((remote_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket failed\n");
		exit(0);
	}
	
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_port = htons(PORT);

	if(inet_pton(AF_INET, "127.0.0.1", &remote_addr.sin_addr) <= 0)
	{
		perror("invalid address/failed to convert\n");
		exit(0);
	}
	
	if(connect(remote_fd, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) < 0)
	{
		perror("failed to connect\n");
		exit(0);
	}


	int i;
	clock_t delay_start;
	double curr_delay;
	for (i = 0; i < 200; i++)
	{
		size_index = rand() % 8;
		time_index = rand() % 14;

		printf("msg size: %d\n", packet_sizes[size_index]);

		send(remote_fd, packets[size_index], packet_sizes[size_index], 0);

		delay_start = clock();
		curr_delay = 0;
		while(curr_delay < delay_times[time_index])
		{
			curr_delay = (double)(clock() - delay_start) / CLOCKS_PER_SEC;
		}

	}

}
