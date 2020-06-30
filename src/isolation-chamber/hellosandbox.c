#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>

static void print_nodename() {
	struct utsname utsname;
	uname(&utsname);
	printf("%s\n", utsname.nodename);
}

int main(int argc, char *argv[]){

	printf("%s %s\n",argv[0], argv[1]);
	printf("hello from sandbox\n");
	//system("pwd");
	//system("ls");
	printf("PID in sandbox: %d\n", getpid());
	printf("hostname in sandbox: ");
	print_nodename();


	return 0;
}