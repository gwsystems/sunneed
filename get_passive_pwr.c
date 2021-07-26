#include <bq27441.h>
#include <stdlib.h>
#include <stdio.h>
#include <wait.h>
#include <time.h>
#include <unistd.h>

#include "bq27441.h"

int
main(void)
{
	int pid;
	double avg_pwr;
	avg_pwr = 0;
	bq27441_init(1);
	if ( (pid = fork()) == 0) {
	    int new_stdin = open("sunneed_out", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH);
	    if (new_stdin == -1) {
		fprintf(stderr, "open err\n");
	    	exit(1);
	    }
	    if (close(1) == -1) {
		fprintf(stderr, "close err\n");
		exit(1);
	    }
	    if (dup2(new_stdin, 1) == -1) {
		fprintf(stderr, "dup err\n");
		exit(1);
	    }
	    execl("build/sunneed", "sunneed", NULL);
	    fprintf(stderr, "execl err: %s\n", strerror(errno));
	    exit(0);
	} else {
	    for (int i = 0; i < 30; i++) {
		avg_pwr += bq27441_average_power();
		printf("avg_pwr: %f, cur_pwr: %d\n", avg_pwr / (i + 1), bq27441_average_power());
		sleep(10);   
	    }
	    avg_pwr = avg_pwr / 30;
	    printf("avg pwr: %f\n", avg_pwr);
	}
	return 0;
}
