#include "sunneed_proc.h"

struct sunneed_tenant tenants[MAX_TENANTS];

int
sunneed_init_tenants(void) {
    for (int i = 0; i < MAX_TENANTS; i++) {
        tenants[i] = (struct sunneed_tenant){.id = i, .pid = 0, .power_proportion = 0.0, .is_active = false};
    }

    return 0;
}

// Find an unused spot for a tenant and register ourself there.
struct sunneed_tenant *
sunneed_tenant_register(pid_t pid) {
    struct sunneed_tenant *tenant = NULL;
    for (int i = 0; i < MAX_TENANTS; i++) {
        if (!tenants[i].is_active) {
            tenant = &tenants[i];
            break;
        }
    }

    if (tenant == NULL) {
        LOG_E("Sorry PID %d, can't spawn any more tenants!", pid);
        return NULL;
    }

    tenant->pid = pid;
    tenant->is_active = true;

    return tenant;
}

int
sunneed_tenant_unregister(struct sunneed_tenant *tenant) {
    if (tenant == NULL || !tenant->is_active) {
        LOG_W("Cannot deactivate an inactive tenant");
        return 1;
    }

    tenant->is_active = false;

    return 0;
}

unsigned int
sunneed_get_num_tenants(void) {
    unsigned int num_tenants = 0;

    for (int i = 0; i < MAX_TENANTS; i++) {
        if (tenants[i].is_active)
            num_tenants++;
    }

    return num_tenants;
}

int
sunneed_update_tenant_cpu_usage(void) {
    // TODO Verify that the pipe and PID match up. It is entirely possible for the tenant process to die and a new
    //  process with the same PID as the tenant's to start up.
    FILE *file;
    char filepath[FILENAME_MAX] = "/proc/stat";

    file = fopen(filepath, "r");
    fscanf(file, "%*s %llu %llu %llu %llu", &cpu_usage.user, &cpu_usage.nice, &cpu_usage.sys, &cpu_usage.idle);
    fclose(file);

    for (struct sunneed_tenant *tenant = tenants; tenant < tenants + MAX_TENANTS; tenant++) {
        if (!tenant->is_active)
            continue;

        snprintf(filepath, FILENAME_MAX, "/proc/%d/stat", tenant->pid);
        if (access(filepath, F_OK) != 0) {
            LOG_E("Unable to find procfs file for PID %d; most likely a tenant ended but forgot to tell us",
                  tenant->pid);
            continue;
        }

        file = fopen(filepath, "r");
        // Read CPU consumption from this tenant's PID.
        fscanf(file,
               "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u" // 13 things we don't care about
               " %llu %llu" // usertime, systemtime
               " %*d %*d %*d %*d %*d %*d %*u %*u", // 8 things we don't care about
               &cpu_usage.tenants[tenant->id].user, &cpu_usage.tenants[tenant->id].sys);
        fclose(file);
    }

    return 0;
}

int
sunneed_get_tenant_cpu_usage(sunneed_tenant_id_t tenant_id) {
    if (!tenants[tenant_id].is_active) {
        LOG_E("Attempt to get CPU usage of inactive tenant %d", tenant_id);
        return -1;
    }

    // TODO Shit

    return 0;
}

sunneed_worker_thread_result_t
sunneed_proc_monitor(__attribute__((unused)) void *args) {
    int ret;
    while (true) {
        LOG_D("Updating process CPU usage");
        if ((ret = sunneed_update_tenant_cpu_usage()) != 0) {
            LOG_E("Error updating CPU usage; monitor thread stopping");
            return NULL;
        }

        sleep(5);
    }
}

sunneed_worker_thread_result_t
sunneed_stepperMotor_driver(__attribute__((unused)) void *args) {
    int status, stepper_new_stdin;
    const char *executable_path = "./ext/SunneeD_dev_drivers/StepperDriver/stepper_driver";
    
    LOG_I("Starting stepper motor driver: tenants can write to /tmp/stepper");
    stepper_new_stdin = open("/tmp/stepper", O_RDONLY);
    mkfifo("/tmp/stepper", S_IRWXU | S_IROTH | S_IWOTH);
    //    stepper_signal_fd = open("/tmp/stepper", O_RDWR | O_CREAT, S_IRWXU | S_IROTH | S_IWOTH);
    if (stepper_new_stdin == -1) {
        if (mkfifo("/tmp/stepper", S_IRWXU | S_IWOTH) == -1) {
	    LOG_E("Could not create FIFO");	
	    return NULL;
	}
	stepper_new_stdin = open("/tmp/stepper", O_RDONLY);
    }

    if (pipe(stepper_dataPipe) == -1) {
	LOG_E("Could not create data pipe to stepper motor driver");
	return NULL;
    }

    if ( (stepper_driver_pid = fork()) == 0) { /* child proc -- stepper motor driver */
	close(stepper_dataPipe[0]);
	if (close(0) == -1) {
	    LOG_E("Error closing stdin for stepper driver: %s\n",strerror(errno));
	    return NULL;
	}
	if (dup2(stepper_new_stdin, 0) == -1) {
	    LOG_E("Error duping stepper_signal_fd: %s\n", strerror(errno));
	    return NULL;
	}
	close(stepper_new_stdin);

	if (close(1) == -1) {
	    LOG_E("Error closing stdin for stepper driver: %s\n",strerror(errno));
	    return NULL;
	}
	if (dup2(stepper_dataPipe[1], 1) == -1) {
	    LOG_E("Error duping stepper_signal_fd: %s\n", strerror(errno));
	    return NULL;
	}
	close(stepper_dataPipe[1]);

	execl(executable_path, executable_path, NULL);
	    LOG_E("Stepper driver could not execute: errno %s", strerror(errno));
   	return NULL; /* shouldn't be reached -- driver runs on infinite loop */
    } else { /* parent (pthread) -- doesn't do anything */
	close(stepper_dataPipe[1]);
	stepper_signal_fd = open("/tmp/stepper", O_WRONLY);
   	wait(&status);
	LOG_E("ERR stepper driver exited with status %d -- errno: %s", status,strerror(errno));
	return NULL;
    }
}
