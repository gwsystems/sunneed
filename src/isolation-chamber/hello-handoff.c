#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]){

	printf("Handoff.c PID: %d\n", getpid());
	char *args[] = {"Hello", "Sandbox", NULL};
	execv("./hellosandbox", args);
	printf("back to handoff.c\n");


	return 0;
}