#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/utsname.h>


int main(int argc, char *argv[]){

	printf("hello from test-namespaces\n");
	
	//UTS
	struct utsname utsname;
	uname(&utsname);
	assert(strcmp(utsname.nodename, "tenant") == 0);

	//PID
	assert(getpid() == 1);

	//NET
	system("ip link");
	//possibly use shell to check output

	//MOUNT
	//try to get parent directory and fail
	//try to access host mounts and fail
	//but most of all, have fun


	printf("Done testing!\n");
	return 0;
}
