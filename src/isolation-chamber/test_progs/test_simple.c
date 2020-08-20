#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/mount.h>
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>

int main(int argc, char *argv[]){

	printf("Hello from test-simple\n");
	return 0;
}
