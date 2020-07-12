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
#include <linux/reboot.h>
#include <sys/reboot.h>
int main(int argc, char *argv[]){

	printf("hello from test 0\n");
	printf("PID in sandbox: %d\n", getpid());

	assert(-1 == mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL));
	assert(-1 == reboot(LINUX_REBOOT_CMD_POWER_OFF));
        
	printf("Done testing!\n");
	return 0;
}
