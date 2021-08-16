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
    int status, stepper_driver_new_stdin;
    const char *path = "/tmp/stepper";
    const char *executable_path = "ext/SunneeD_dev_drivers/StepperDriver/stepper_driver";

    if (pipe(stepper_dataPipe) == -1) {
	LOG_E("Could not create data pipe for stepper driver");
	exit(1);
    }

    /* initialize variables used in serve_write for pwr recordings */
    sunneed_stepperDir = STOPPED;
    last_stepperMotor_req_time = NULL;

    if ( (sunneed_stepper_driver_pid = fork()) == 0) {
        /* set new proc's stdin to stepper motor device file, stdout to sunneed's communication pipe */
        close(stepper_dataPipe[0]);

	if (mkfifo(path, S_IRUSR | S_IWUSR | S_IWOTH) == -1) {
            /* check if pipe already exists */
            if ((stepper_driver_new_stdin = open(path, O_RDONLY)) == -1) {
                /* cound't create pipe & it doesn't already exist */
                LOG_E("Error creating pipe to stepper driver");
                exit(1);
            }
        } else {
            if ((stepper_driver_new_stdin = open(path, O_RDONLY)) == -1) {
                /* pipe exists, couldn't open it */
                LOG_E("Error opening pipe to stepper driver");
                exit(1);
            }
        }
        
        if (close(0) == -1) {
            LOG_E("error closing stdin for stepper driver");
            exit(1);
        }
        if (dup2(stepper_driver_new_stdin, 0) == -1) {
            LOG_E("error duping new stdin for stepper driver");
            exit(1);
        }
        if (close(stepper_driver_new_stdin) == -1) {
            LOG_E("error closing stepper driver's 2nd ref to new stdin after dup");
            exit(1);
        }
	if (close(1) == -1) {
	    LOG_E("Error closing stdout for stepper driver");
	    exit(1);
	}
	if (dup2(stepper_dataPipe[1], 1) == -1) {
	    LOG_E("Error duping data pipe to stepper driver's stdout");
	    exit(1);
	}
	if (close(stepper_dataPipe[1]) == -1) {
	    LOG_E("Error closing 2nd ref to stepper driver data pipe");
	    exit(1);
	}
        execl(executable_path, executable_path, NULL);
        /* should never get here -- stepper driver runs on loop */
        exit(1);
    } else {
	close(stepper_dataPipe[1]);
        wait(&status);
        LOG_E("Stepper driver exited - status=%d, errno=%s", status, strerror(errno));
        return NULL;
    }
}

sunneed_worker_thread_result_t
sunneed_camera_driver(__attribute__((unused)) void *args) {
    int wait_status, cam_driver_new_stdin;
    const char *path = "/tmp/camera";
    const char *executable_path = "run_PyDriver";

    if ( (sunneed_camera_driver_pid = fork()) == 0) {
    /* set new proc's stdin to camera device file, stdout to sunneed's communication pipe */

        if ( (cam_driver_new_stdin = open(path, O_RDONLY)) == -1) {
            if (mkfifo(path, S_IRUSR | S_IWUSR | S_IWOTH) == -1) {
                LOG_E("Could not create camera device fd");
                exit(1);
            }
            if (chmod(path, S_IRUSR | S_IWUSR | S_IWOTH) == -1) {
                LOG_E("Could not set permissions for camera device fd");
                exit(1);
            }
            cam_driver_new_stdin = open(path, O_RDONLY);
        }
        if (close(0) == -1) {
            LOG_E("Error closing camera driver stdin");
            exit(1);
        }
        if (dup2(cam_driver_new_stdin, 0) == -1) {
            LOG_E("Error duping camera device fd to camera driver stdin");
            exit(1);
        }
        if (close(cam_driver_new_stdin) == -1) {
            LOG_E("Error closing camera driver's unused reference to camera fd");
            exit(1);
        }
        execl(executable_path, executable_path, NULL);

        exit(1);
    } else {
        wait(&wait_status);
        LOG_E("Error executing camera driver: status=%d\n",wait_status);
        return NULL;
    }
}
