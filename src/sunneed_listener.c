#include "sunneed_listener.h"

#include "protobuf/c/server.pb-c.h"

#define SUB_RESPONSE_BUF_SZ 4096

extern struct sunneed_device devices[];
extern struct sunneed_tenant tenants[];
extern const char *locked_file_paths[];

nng_socket sock;

/** 
 * Maps dummy paths (typically sent by clients during a read or write) to FDs pointing to the real device, held by
 * sunneed.
 */
struct {
    char *path;
    int fd;
} dummy_path_fd_map[MAX_TENANTS][MAX_LOCKED_FILES] = {{ { NULL, 0 } }};

static int
get_fd_from_dummy_path(char *path, struct sunneed_tenant *tenant) {
    for (int i = 0; i < MAX_LOCKED_FILES; i++) {
        if (dummy_path_fd_map[tenant->id][i].path && strncmp(dummy_path_fd_map[tenant->id][i].path, path, strlen(path)) == 0)
            return dummy_path_fd_map[tenant->id][i].fd;
    }
    return -1;
}

// Control flow:
// When a new pipe connects, we use this struct to make a mapping of its pipe ID to a tenant. Then, when further
//  requests are made, the pipe ID is used to identify a tenant to the request.
// The client will have to send some notification in order to unregister; I don't think we can tell if a pipe
//  closed.
struct tenant_pipe {
    struct sunneed_tenant *tenant;
    nng_pipe pipe;
} tenant_pipes[SUNNEED_MAX_IPC_CLIENTS];

// TODO This is probably slow -- O(n) lookup for every request made.
static struct sunneed_tenant *
tenant_of_pipe(int pipe_id) {
    for (int i = 0; i < SUNNEED_MAX_IPC_CLIENTS; i++)
        if (nng_pipe_id(tenant_pipes[i].pipe) == pipe_id)
            return tenant_pipes[i].tenant;
    return NULL;
}

// Get the PID of a pipe and use that to create a new sunneed tenant with that ID.
// TODO: This shouldn't always create a new tenant, since we want multiple processes
//  mapped to one tenant.
static struct sunneed_tenant *
register_client(nng_pipe pipe) {
    struct sunneed_tenant *tenant;

    // Get PID of pipe.
    uint64_t pid_int;
    SUNNEED_NNG_TRY(nng_pipe_get_uint64, != 0, pipe, NNG_OPT_IPC_PEER_PID, &pid_int);
    pid_t pid = (pid_t)pid_int;

    if ((tenant = sunneed_tenant_register(pid)) == NULL) {
        LOG_E("Failed to initialize tenant from PID %d", pid); 
        return NULL;
    }

    for (int i = 0; i < SUNNEED_MAX_IPC_CLIENTS; i++) {
        if (tenant_pipes[i].tenant == NULL) {
            tenant_pipes[i].tenant = tenant;
            tenant_pipes[i].pipe = pipe;
            break;
        }
    }

    return tenant;
}

// The `serve_*` methods take a `sub_resp_buf` parameter. This is a pointer to a buffer in which the client
//  can store their sub-response (the message in the oneof field of the SunneedResponse). Example:
//
//    GetDeviceHandleResponse *sub_resp = sub_resp_buf;
//    *sub_resp = (GetDeviceHandleResponse)GET_DEVICE_HANDLE_RESPONSE__INIT;
//
// This example writes the initializer for the `GetDeviceHandleResponse` to the address pointed to by
//  `sub_resp_buf`.
// The rationale for this whole process comes next: once the `serve_*` function returns, its sub-response
//  data is contained within the buffer, to which a pointer is in scope in the main request listening
//  loop.

// Create a mapping between this pipe and a sunneed tenant.
// TODO Currently, this just spawns a new tenant for each different pipe. We want tenants to be able to have multiple 
//  pipes to sunneed open.
static int
serve_register_client(SunneedResponse *resp, void *sub_resp_buf, nng_pipe pipe) {
    resp->message_type_case = SUNNEED_RESPONSE__MESSAGE_TYPE_REGISTER_CLIENT;
    RegisterClientResponse *sub_resp = sub_resp_buf;
    *sub_resp = (RegisterClientResponse)REGISTER_CLIENT_RESPONSE__INIT;
    resp->register_client = sub_resp;

    struct sunneed_tenant *tenant = NULL;

    // Register as a new client.
    if ((tenant = register_client(pipe)) == NULL) {
        LOG_W("Registration failed for pipe %d", pipe.id);
        return 1;
    }

    // Construct the list of locked file paths to send to the client.
    size_t locked_paths_len = 0;
    for (locked_paths_len = 0; locked_file_paths[locked_paths_len] != NULL; locked_paths_len++) ;
    sub_resp->n_locked_paths = locked_paths_len;
    sub_resp->locked_paths = malloc(sizeof(char *) * sub_resp->n_locked_paths);
    for (size_t i = 0; i < sub_resp->n_locked_paths; i++)
        sub_resp->locked_paths[i] = (char *)locked_file_paths[i];

    LOG_D("Registered pipe %d with tenant %d", pipe.id, tenant->id);

    return 0;
}

static int
serve_unregister_client(SunneedResponse *resp, void *sub_resp_buf, nng_pipe pipe, struct sunneed_tenant *tenant) {
    int retval;

    LOG_D("Unregistering tenant %d", tenant->id);

    // Find the entry in the tenant pipe mappings.
    bool cleared = false;
    for (int i = 0; i < SUNNEED_MAX_IPC_CLIENTS; i++)
        if (tenant_pipes[i].pipe.id == pipe.id) {
            LOG_D("Removing mapping from pipe %d to tenant %d", pipe.id, tenant->id); 
            tenant_pipes[i].tenant = NULL;
            tenant_pipes[i].pipe = (nng_pipe)NNG_PIPE_INITIALIZER;
            cleared = true;
            break;
        }

    if (!cleared) {
        LOG_E("No mapping cleared when unregistering pipe %d; something is wrong with the pipe->tenant table", pipe.id);
        return 1;
    }

    if ((retval = sunneed_tenant_unregister(tenant)) != 0)
        // TODO Handle (follow the pattern of the `register_client` stuff by making a secondary `unregister` function
        //  that handles interacting with tenants).
        return 1;

    /* clear dummy_path -> fd map for when tenant is re-activated */
    for (int i = 0; i < MAX_LOCKED_FILES; i++) {
        dummy_path_fd_map[tenant->id][i].path = NULL;
        dummy_path_fd_map[tenant->id][i].fd = 0;
    }


    resp->message_type_case = SUNNEED_RESPONSE__MESSAGE_TYPE_GENERIC;
    GenericResponse *sub_resp = sub_resp_buf;
    *sub_resp = (GenericResponse)GENERIC_RESPONSE__INIT;
    resp->generic = sub_resp;

    return 0;
}

static int
serve_open_file(
        SunneedResponse *resp,
        void *sub_resp_buf,
        __attribute__((unused)) struct sunneed_tenant *tenant,
        OpenFileRequest *request) {
    LOG_D("Got request to open file '%s'", request->path);

    OpenFileResponse *sub_resp = sub_resp_buf;
    *sub_resp = (OpenFileResponse)OPEN_FILE_RESPONSE__INIT;
    resp->message_type_case = SUNNEED_RESPONSE__MESSAGE_TYPE_OPEN_FILE;
    resp->open_file = sub_resp;

    struct sunneed_device *locker;
    if ((locker = sunneed_device_file_locker(request->path)) != NULL) {
        // TODO Wait for availability, perform power calcs, etc.

        // Open the real file and save its FD.
        int real_fd = open(request->path, request->flags, request->mode); // TODO Use flags given by client.
        
        if (real_fd == -1) {
            LOG_E("Failed to open file '%s'", request->path);
            return 1;
        }
        
        char *dummypath = sunneed_device_get_dummy_file(request->path);

        int i;
        for (i = 0; i < MAX_LOCKED_FILES; i++) {
            // Find open slot.
            if (dummy_path_fd_map[tenant->id][i].path == NULL) {
                dummy_path_fd_map[tenant->id][i].path = malloc(strlen(dummypath) + 1);
                strncpy(dummy_path_fd_map[tenant->id][i].path, dummypath, strlen(dummypath) + 1);
                dummy_path_fd_map[tenant->id][i].fd = real_fd;

                LOG_I("Opened locked path '%s' as '%s' (FD %d)", request->path, dummypath, real_fd);

                break;
            }
        }
        if (i == MAX_LOCKED_FILES) {
            // Theoretically this should never happen (since MAX_LOCKED_FILES also bounds the number of possible locked
            //  paths) but good to check.
            LOG_E("No slots remaining in dummy_path_fd_map");
            return 1;
        }
        
        // TODO Free this
        sub_resp->path = malloc(strlen(dummypath) + 1);
        strncpy(sub_resp->path, dummypath, strlen(dummypath) + 1);
    } else {
        // They requested a non-dummy file.
        return 1;
    }

    return 0;
}

static int
serve_close(
        SunneedResponse *resp,
        void *sub_resp_buf,
        struct sunneed_tenant *tenant,
        CloseFileRequest *request) {
    LOG_D("Got request from %d to close '%s' (real file FD %d)", tenant->id, request->dummy_path, get_fd_from_dummy_path(request->dummy_path, tenant));

    CloseFileResponse *sub_resp = sub_resp_buf;
    *sub_resp = (CloseFileResponse)CLOSE_FILE_RESPONSE__INIT;
    resp->message_type_case = SUNNEED_RESPONSE__MESSAGE_TYPE_CLOSE_FILE;
    resp->close_file= sub_resp;

    for (int i = 0; i < MAX_LOCKED_FILES; i++) {
        if (dummy_path_fd_map[tenant->id][i].path && strncmp(dummy_path_fd_map[tenant->id][i].path, request->dummy_path, strlen(request->dummy_path)) == 0) {
            if (close(dummy_path_fd_map[tenant->id][i].fd) < 0) {
                int errno_val = errno;
                sub_resp->errno_value = errno_val;
                LOG_E("'close' for client %d failed with: %s", tenant->id, strerror(errno));
                return 1;
            }
            /* reset array spot to be reused on future serve_open() calls */
            dummy_path_fd_map[tenant->id][i].path = NULL;
            dummy_path_fd_map[tenant->id][i].fd = 0;

            sub_resp->errno_value = 0;
            return 0;
        }
    }
    return 1;
}

static int
serve_write(
        SunneedResponse *resp,
        void *sub_resp_buf,
        struct sunneed_tenant *tenant,
        WriteRequest *request) {
    LOG_D("Got request from %d to write %ld bytes to '%s' (real file FD %d)", tenant->id, request->data.len, request->dummy_path, get_fd_from_dummy_path(request->dummy_path, tenant));

    WriteResponse *sub_resp = sub_resp_buf;
    *sub_resp = (WriteResponse)WRITE_RESPONSE__INIT;
    resp->message_type_case = SUNNEED_RESPONSE__MESSAGE_TYPE_CALL_WRITE;
    resp->call_write = sub_resp;
    #ifdef LOG_PWR
    char *real_path = get_path_from_dummy_path(request->dummy_path);
    int num_pwr_readings;
    float request_n_sec /* how long device was drawing extra power to service request in seconds */;
    float avg_pwr;
    float time_since_pwrRead;

    /*
     * Stepper motor specific
     */
    char stepper_sig; /* used in recording pwr used by stepper motor - stepper driver sends signal when request is completed by the device */
    bool change_dir, from_stop; 
    int orientation_change;
    
    
    struct timespec *curr_time, *request_start_time, *request_end_time;
    curr_time  = (struct timespec*) malloc(sizeof(struct timespec));
    request_start_time = (struct timespec*) malloc(sizeof(struct timespec));
    request_end_time = (struct timespec*) malloc(sizeof(struct timespec));
    clock_gettime(CLOCK_BOOTTIME, curr_time);
    clock_gettime(CLOCK_BOOTTIME, request_start_time);
    
    if (strcmp(real_path, "/tmp/stepper") == 0) {
        char orientation_bytes[request->data.len];

        if (stepperMotor_orientation == -1) {
            /* TODO: read orientation file */
            stepperMotor_orientation = 0;
        }

        change_dir = from_stop = false;
    
        if (request->data.data[0] == '+' || request->data.data[0] == '-') {
            strncpy(orientation_bytes, (char*)(request->data.data + 1), request->data.len - 1);
            orientation_bytes[request->data.len - 1] = '\0';
            orientation_change = atoi(orientation_bytes);                

            if (request->data.data[0] == '+') {
                if (sunneed_stepperDir == COUNTER_CLOCKWISE) {
                    change_dir = true;
                }
                sunneed_stepperDir = CLOCKWISE;
                stepperMotor_orientation += orientation_change;
            } else {
                if (sunneed_stepperDir == CLOCKWISE) {
                    change_dir = true;
                }
                sunneed_stepperDir = COUNTER_CLOCKWISE;
                stepperMotor_orientation -= orientation_change;
            }
        } else {
            int new_orientation = ((int)request->data.data[1] << 8) | (int) request->data.data[0];
            orientation_change = abs(stepperMotor_orientation - new_orientation); 
            stepperMotor_orientation = new_orientation;
        }
        if (last_stepperMotor_req_time != NULL) {
            if ( ((curr_time->tv_sec - last_stepperMotor_req_time->tv_sec) + ((curr_time->tv_nsec - last_stepperMotor_req_time->tv_nsec) / 10e9) ) > 0.5) {
                from_stop = true;
                change_dir = false;
            }
        } else {
            /* first request to stepper motor -- initialize variables */
            last_stepperMotor_req_time = (struct timespec*) malloc(sizeof(struct timespec));
            clock_gettime(CLOCK_BOOTTIME, last_stepperMotor_req_time);
            last_stepperMotor_req_time->tv_sec--; /*decrement 1s so we don't wait before first power reading */
            from_stop = true;
            change_dir = false;
        }
    }
    #endif

    // Perform the write.
    ssize_t bytes_written;
    if ((bytes_written = write(get_fd_from_dummy_path(request->dummy_path, tenant), request->data.data, request->data.len)) 
            < 0) {
        int errno_val = errno;

        sub_resp->errno_value = errno_val;
        sub_resp->bytes_written = bytes_written;

        LOG_E("`write` for client %d failed with: %s", tenant->id, strerror(errno_val));

        free(curr_time);
        free(request_end_time);
        free(request_start_time);
        return 1;
    }
    
    sub_resp->bytes_written = bytes_written;
    sub_resp->errno_value = 0;

    #ifdef LOG_PWR
    if (strcmp(real_path, "/tmp/stepper") == 0) {
        

        LOG_I("Waiting for stepper driver to finish");
        stepper_sig = 'z';
        struct timespec *last_pwrRead_time = (struct timespec*) malloc(sizeof(struct timespec));
        clock_gettime(CLOCK_BOOTTIME, curr_time);
	    clock_gettime(CLOCK_BOOTTIME, last_pwrRead_time);
	    time_since_pwrRead = (curr_time->tv_sec - last_stepperMotor_req_time->tv_sec + (10e-9 * (curr_time->tv_nsec - last_stepperMotor_req_time->tv_nsec))); 
        if (time_since_pwrRead >= 1) {
            /* need min 1s between reads of battery babysitter power measurement */
            avg_pwr = present_power() - PASSIVE_PWR;
        } else {
            usleep(10e3 * ( 1 - time_since_pwrRead));
            avg_pwr = present_power() - PASSIVE_PWR;
            clock_gettime(CLOCK_BOOTTIME, last_pwrRead_time);
        }
        
        num_pwr_readings = 1;
        
        do { /* get average power draw from battery while stepper motor served request */
            clock_gettime(CLOCK_BOOTTIME, curr_time);
            read(stepper_dataPipe[0], &stepper_sig, 1);
            if ((curr_time->tv_sec - last_pwrRead_time->tv_sec + (10e-9 * (curr_time->tv_nsec - last_pwrRead_time->tv_nsec))) >= 1 /* poll rate for bq27441_average_power() is 1 s */) {
                avg_pwr += (present_power() - PASSIVE_PWR);
                num_pwr_readings++;
                clock_gettime(CLOCK_BOOTTIME, last_pwrRead_time);
            }
        } while (stepper_sig != 'a'); /* wait for stepper motor to finish turning */
        stepper_sig = 'z';

        avg_pwr = avg_pwr / num_pwr_readings;
        
        clock_gettime(CLOCK_BOOTTIME, last_stepperMotor_req_time);
        clock_gettime(CLOCK_BOOTTIME, request_end_time);

        request_n_sec = ((float)(request_end_time->tv_sec - request_start_time->tv_sec) + ( ((float)request_end_time->tv_nsec - (float)request_start_time->tv_nsec) * 10e-10));
	    /* I have no idea why this is e-10 and not e-9 ... 1ns = 10e-9s, but this always gives magnitude too high  */

        LOG_D("%d, %d, %d, %f",orientation_change, change_dir, from_stop, avg_pwr * request_n_sec);
        LOG_P("%d, %d, %d, %f\n",orientation_change, change_dir, from_stop, avg_pwr * request_n_sec);
    
        free(curr_time);
        free(request_end_time);
        free(request_start_time);
        free(last_pwrRead_time);
    }
    #endif
    return 0;
}

static void
report_nng_error(const char *func, int rv) {
    LOG_E("nng error: (%s) %s", func, nng_strerror(rv));
}

int
sunneed_listen(void) {
    SUNNEED_NNG_SET_ERROR_REPORT_FUNC(report_nng_error);

    // Initialize client states.
    for (int i = 0; i < MAX_TENANTS; i++) {
        tenant_pipes[i] = (struct tenant_pipe){.tenant = NULL,
                                                 // TODO Why do I need to cast this...
                                                 .pipe = (nng_pipe)NNG_PIPE_INITIALIZER};
    }


    LOG_I("Starting listener loop...");

    // Make a socket and attach it to the sunneed URL.
    SUNNEED_NNG_TRY_RET(nng_rep0_open, != 0, &sock);
    SUNNEED_NNG_TRY_RET(nng_listen, < 0, sock, SUNNEED_LISTENER_URL, NULL, 0);

    // Await messages.
    for (;;) {
        nng_msg *msg;
	LOG_D("start listener loop");
  	SUNNEED_NNG_TRY_RET(nng_recvmsg, != 0, sock, &msg, NNG_FLAG_ALLOC);
	LOG_D("got msg");
        // TODO They claim nng_msg_get_pipe() returns -1 on error, but its return type is nng_pipe, which can't
        //  be compared to an integer.
        nng_pipe pipe = nng_msg_get_pipe(msg);

        // Get contents of message.
        size_t msg_len = nng_msg_len(msg);


        SunneedRequest *request = sunneed_request__unpack(NULL, msg_len, nng_msg_body(msg));

	LOG_D("unpacked msg");
        if (request == NULL) {
            LOG_W("Received null request from %d", pipe.id);
            goto end;
        }

        struct sunneed_tenant *tenant = NULL;

        // Find the pipe's associated tenant. If we can't find it, we error out unless the message is of type REGISTER_CLIENT.
        if ((tenant = tenant_of_pipe(pipe.id)) == NULL && request->message_type_case != SUNNEED_REQUEST__MESSAGE_TYPE_REGISTER_CLIENT) {
            // This client has not registered!
            LOG_W("Received message from %d, who is not registered.", pipe.id);
            goto end;
        }
	LOG_D("got tenant from pipe");

    schedule_request(request, tenant, pipe, 0); /* TODO: insert actual power estimate for request instead of 0 */

    end:
        nng_msg_free(msg);
    }

}

sunneed_worker_thread_result_t
sunneed_request_servicer(__attribute__((unused)) void *args) {
    struct SunneedRequest_ListNode *requestNode;
    struct sunneed_tenant *tenant;
    SunneedRequest *request_to_serve;
    nng_pipe tenant_pipe;
    void *sub_resp_buf = malloc(SUB_RESPONSE_BUF_SZ);

    LOG_I("Starting request servicer");

    if (sub_resp_buf == NULL) {
        LOG_E("Could not allocate subresponse buffer");
        LOG_I("Sunneed exiting");
        LOG_I("\tKilling stepper motor");
        kill(sunneed_stepper_driver_pid, SIGTERM);
        LOG_I("\tKilling camera driver");
        kill(sunneed_camera_driver_pid, SIGTERM);
        abort();
    }
    while (true) {
        if (sunneed_queued_requests.num_active_requests > 0) {
            requestNode = pop_requestNode();
            if (requestNode == NULL) continue;
            request_to_serve = requestNode->request;
            tenant = requestNode->tenant;
            tenant_pipe = requestNode->tenant_pipe;

            SunneedResponse resp = SUNNEED_RESPONSE__INIT;
            int ret = -1;
            
            switch (request_to_serve->message_type_case) {
                case SUNNEED_REQUEST__MESSAGE_TYPE__NOT_SET:
                    LOG_W("Request from pipe %d has no message type set.", tenant_pipe.id);
                    ret = -1;
                    break;
                case SUNNEED_REQUEST__MESSAGE_TYPE_REGISTER_CLIENT:
                    ret = serve_register_client(&resp, sub_resp_buf, tenant_pipe);
                    break;
                case SUNNEED_REQUEST__MESSAGE_TYPE_UNREGISTER_CLIENT:
                    ret = serve_unregister_client(&resp, sub_resp_buf, tenant_pipe, tenant);
                    break;
                case SUNNEED_REQUEST__MESSAGE_TYPE_OPEN_FILE:
                    ret = serve_open_file(&resp, sub_resp_buf, tenant, request_to_serve->open_file);
                    break;
                case SUNNEED_REQUEST__MESSAGE_TYPE_WRITE:
                    ret = serve_write(&resp, sub_resp_buf, tenant, request_to_serve->write);
                    break;
                case SUNNEED_REQUEST__MESSAGE_TYPE_CLOSE_FILE:
                    ret = serve_close(&resp, sub_resp_buf, tenant, request_to_serve->close_file);
                    break;
                default:
                    LOG_W("Received request with invalid type %d", request_to_serve->message_type_case);
                    ret = -1;
                    break;
            }

            free(requestNode);

            resp.status = ret;
            // Create and send the response message.
            nng_msg *resp_msg;
            int resp_len = sunneed_response__get_packed_size(&resp);
            void *resp_buf = malloc(resp_len);
            sunneed_response__pack(&resp, resp_buf);

            SUNNEED_NNG_TRY(nng_msg_alloc, != 0, &resp_msg, 0);
            SUNNEED_NNG_TRY(nng_msg_append, != 0, resp_msg, resp_buf, resp_len);
            SUNNEED_NNG_TRY(nng_sendmsg, != 0, sock, resp_msg, 0);

            sunneed_request__free_unpacked(request_to_serve, NULL);
        }
    }
}
