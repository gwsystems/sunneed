//TODO better macro names
#define SLEEP_PERIOD 5 //seconds of sleep between reads
#define QUANTUM_PERIOD 10 //minutes per quantum
#define READS_PER_QUANTUM ((QUANTUM_PERIOD*60)/SLEEP_PERIOD)//reads per quantum 

#include <bq27441.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>

#include <unistd.h>

void i2c_read(uint16_t * voltage, int16_t * current, uint16_t * capacity, uint16_t * state_of_charge){
    *voltage = bq27441_voltage();
    *current = bq27441_average_current();
    *capacity = bq27441_nominal_avail_cap();
    *state_of_charge = bq27441_state_of_charge();

}


void quant_eval(uint16_t voltage[], int16_t current[], uint16_t capacity[], uint16_t state_of_charge[]){
    uint16_t capacity_drop = capacity[0]-capacity[READS_PER_QUANTUM-1];
    int16_t total_current = 0;
    int i;
    for (i=0; i<READS_PER_QUANTUM; i++){
        total_current+=current[i];
    }
    int avg_current = total_current/READS_PER_QUANTUM;
    //print logic
    FILE *quant = fopen("quantum.txt", "w"); 
    if(quant != NULL){
        fprintf(quant,"Avg Current for this quantum is %i mA\n",avg_current);
        fprintf(quant,"Capacity drop for this quantum is %i mAh\n",capacity_drop);
    }
    fclose(quant);

}


int main(void){
    int i =0;
    uint16_t voltage[READS_PER_QUANTUM];
    int16_t current[READS_PER_QUANTUM];
    uint16_t capacity[READS_PER_QUANTUM];
    uint16_t state_of_charge[READS_PER_QUANTUM];
    if (bq27441_init(1)!=0){
        return 1;
    }
    while(1)
    {
        i2c_read(&voltage[i],&current[i],&capacity[i],&state_of_charge[i]);
        sleep(SLEEP_PERIOD);
        i++;
        if (i%READS_PER_QUANTUM == 0){
            i = 0;
            quant_eval(voltage,current,capacity,state_of_charge);
        }    
    }
}

