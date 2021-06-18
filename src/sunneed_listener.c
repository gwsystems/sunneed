#include "sunneed_listener.h"

#include "protobuf/c/server.pb-c.h"

#define SUB_RESPONSE_BUF_SZ 4096

extern struct sunneed_device devices[];
extern struct sunneed_tenant tenants[];
extern const char *locked_file_paths[];

/** 
 * Maps dummy paths (typically sent by clients during a read or write) to FDs pointing to the real device, held by
 * sunneed.
 */
struct {
    char *path;
    int fd;
} dummy_path_fd_map[MAX_LOCKED_FILES] = { { NULL, 0 } };

struct {
    int id;
    int sockfd;
    int domain;
} dummy_socket_map[MAX_TENANT_SOCKETS] = { {0, 0, 0} };

static int
get_fd_from_dummy_path(char *path) {
    for (int i = 0; i < MAX_LOCKED_FILES; i++)
        if (dummy_path_fd_map[i].path && strncmp(dummy_path_fd_map[i].path, path, strlen(path)) == 0)
            return dummy_path_fd_map[i].fd;
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

    // TODO Take flags!!


    struct sunneed_device *locker;
    if ((locker = sunneed_device_file_locker(request->path)) != NULL) {
        // TODO Wait for availability, perform power calcs, etc.

        // Open the real file and save its FD.
//        int real_fd = open(request->path, O_RDWR); // TODO Use flags given by client.
	int real_fd = open(request->path, request->flags);

	if (real_fd == -1) {
	    LOG_E("Failed to open file '%s'", request->path);
	    return 1;
	}

	char *dummypath = sunneed_device_get_dummy_file(request->path);

        int i;
        for (i = 0; i < MAX_LOCKED_FILES; i++) {
            // Find open slot.
            if (dummy_path_fd_map[i].path == NULL) {
                dummy_path_fd_map[i].path = malloc(strlen(dummypath) + 1);
                strncpy(dummy_path_fd_map[i].path, dummypath, strlen(dummypath) + 1);
                dummy_path_fd_map[i].fd = real_fd;

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
        sub_resp->path = malloc(strlen(dummypath));
        strncpy(sub_resp->path, dummypath, strlen(dummypath) + 1);
    } else {
        // They requested a non-dummy file.
        return 1;
    }

    return 0;
}

static int
serve_write(
        SunneedResponse *resp,
        void *sub_resp_buf,
        struct sunneed_tenant *tenant,
        WriteRequest *request) {
    LOG_D("Got request from %d to write %ld bytes to '%s' (real file FD %d)", tenant->id, request->data.len, request->dummy_path, get_fd_from_dummy_path(request->dummy_path));

    WriteResponse *sub_resp = sub_resp_buf;
    *sub_resp = (WriteResponse)WRITE_RESPONSE__INIT;
    resp->message_type_case = SUNNEED_RESPONSE__MESSAGE_TYPE_CALL_WRITE;
    resp->call_write = sub_resp;

    // Perform the write.
    ssize_t bytes_written;
    if ((bytes_written = write(get_fd_from_dummy_path(request->dummy_path), request->data.data, request->data.len)) 
            < 0) {
        int errno_val = errno;

        sub_resp->errno_value = errno_val;
        sub_resp->bytes_written = bytes_written;

        LOG_E("`write` for client %d failed with: %s", tenant->id, strerror(errno_val));

        return 1;
    }

    sub_resp->bytes_written = bytes_written;
    sub_resp->errno_value = 0;

    return 0;
}

static int
serve_socket(SunneedResponse *resp, void *sub_response_buf, SocketRequest *request)
{
	LOG_D("Got request to open a new socket\n");

	SocketResponse *sock_resp = sub_response_buf;
	*sock_resp = (SocketResponse)SOCKET_RESPONSE__INIT;
	resp->message_type_case = SUNNEED_RESPONSE__MESSAGE_TYPE_SOCKET;
	resp->socket = sock_resp;
	//TODO: checks on the request?
	
	//find open spot in socket table
	int i, new_id, sockfd;
	for(i = 0; i < MAX_TENANT_SOCKETS; i++)
	{
		new_id = i;
		if(dummy_socket_map[i].id == 0)
		{
			sockfd = socket(request->domain, request->type, request->protocol);
			if(sockfd)
			{
				dummy_socket_map[i].id = new_id;
				dummy_socket_map[i].sockfd = sockfd;
				dummy_socket_map[i].domain = request->domain;
				LOG_D("Socket created successfully\n");
				break;
			}else{
				LOG_E("Failed to create socket. domain %d type %d protocol %d\n", request->domain, request->type, request->protocol);
				return 1;
			}
		}
	}

	if(new_id == MAX_TENANT_SOCKETS)
	{
		LOG_E("no more sockets can be created\n");
		sock_resp->dummy_sockfd = -1;
		return 1;
	}

	sock_resp->dummy_sockfd = new_id;
	return 0;
}

static int
serve_connect(nng_pipe pipe, ConnectRequest *request)
{
	//lookup real sockfd in dummy_socket_map and create new socket
	LOG_D("got connect request\n");
	int i, sockfd, domain;
	struct sockaddr_in remote_addr;
	sockfd = 0;
	for(i = 0; i < MAX_TENANT_SOCKETS; i++)
	{
		if(dummy_socket_map[i].id == request->sockfd)
		{
			LOG_D("found socket for pipe %d\n", pipe.id);
			sockfd = dummy_socket_map[i].sockfd;
			domain = dummy_socket_map[i].domain;
			break;
		}
	}
	if(!(sockfd))
	{
		LOG_E("failed to find socket for pipe %d\n", pipe.id);
		return 1;
	}

	remote_addr.sin_family = domain;
	remote_addr.sin_port = htons(request->port);

	//TODO: check address/port
	if(inet_pton(domain, request->address, &remote_addr.sin_addr) <= 0)
	{
		LOG_E("invalid address/domain or failed to convert\n");
		return 1;
	}

	if(connect(sockfd, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) < 0)
	{
		LOG_E("Failed to connect to %s\n", request->address);
		 return 1;
	}

	LOG_D("connected to remote host: %s\n", request->address);

	return 0;


}

static int 
serve_send(struct sunneed_tenant *tenant, SendRequest *request)
{
	//TODO: formulate response, for now just log and call send
	
	LOG_D("Got request from %d to send %ld bytes", tenant->id, sizeof(request->data.len));

	//TODO: probably want more checks here as well
	
	int sockfd = request->sockfd;
	if(!(sockfd))
	{
		LOG_E("Bad socket descriptor: %d\n", sockfd);
		return 1;
	}
	if(!(request->data.data))
	{
		LOG_E("couldnt get data from request\n");
		return 1;
	}
	send(sockfd, request->data.data, request->data.len, request->flags);

	LOG_D("Sent data from tenant %d\n", tenant->id);

	return 0;
}
static void
report_nng_error(const char *func, int rv) {
    LOG_E("nng error: (%s) %s", func, nng_strerror(rv));
}

int
sunneed_listen(void) {
    SUNNEED_NNG_SET_ERROR_REPORT_FUNC(report_nng_error);

    #ifdef LOG_PWR
        int capacity_change;
    #endif

    // Initialize client states.
    for (int i = 0; i < MAX_TENANTS; i++) {
        tenant_pipes[i] = (struct tenant_pipe){.tenant = NULL,
                                                 // TODO Why do I need to cast this...
                                                 .pipe = (nng_pipe)NNG_PIPE_INITIALIZER};
    }

    nng_socket sock;

    LOG_I("Starting listener loop...");

    // Make a socket and attach it to the sunneed URL.
    SUNNEED_NNG_TRY_RET(nng_rep0_open, != 0, &sock);
    SUNNEED_NNG_TRY_RET(nng_listen, < 0, sock, SUNNEED_LISTENER_URL, NULL, 0);

    // Buffer for `serve_` methods to write their sub-response to.
    void *sub_resp_buf = malloc(SUB_RESPONSE_BUF_SZ);
    // TODO Check malloc.

    // Await messages.
    for (;;) {
        nng_msg *msg;

        SUNNEED_NNG_TRY_RET(nng_recvmsg, != 0, sock, &msg, NNG_FLAG_ALLOC);

        // TODO They claim nng_msg_get_pipe() returns -1 on error, but its return type is nng_pipe, which can't
        //  be compared to an integer.
        nng_pipe pipe = nng_msg_get_pipe(msg);

        // Get contents of message.
        size_t msg_len = nng_msg_len(msg);
        SUNNEED_NNG_MSG_LEN_FIX(msg_len);
        SunneedRequest *request = sunneed_request__unpack(NULL, msg_len, nng_msg_body(msg));

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

        // Begin setting up our response.
        SunneedResponse resp = SUNNEED_RESPONSE__INIT;
        int ret = -1;

        #ifdef LOG_PWR
            reqs_since_last_log++;
        #endif

        switch (request->message_type_case) {
            case SUNNEED_REQUEST__MESSAGE_TYPE__NOT_SET:
                LOG_W("Request from pipe %d has no message type set.", pipe.id);
                ret = -1;
                break;
            case SUNNEED_REQUEST__MESSAGE_TYPE_REGISTER_CLIENT:
                ret = serve_register_client(&resp, sub_resp_buf, pipe);
                break;
            case SUNNEED_REQUEST__MESSAGE_TYPE_UNREGISTER_CLIENT:
                ret = serve_unregister_client(&resp, sub_resp_buf, pipe, tenant);
                break;
            case SUNNEED_REQUEST__MESSAGE_TYPE_OPEN_FILE:
		ret = serve_open_file(&resp, sub_resp_buf, tenant, request->open_file);
                break;
            case SUNNEED_REQUEST__MESSAGE_TYPE_WRITE:
                #ifdef LOG_PWR
                    if (reqs_since_last_log < REQS_PER_LOG) {
                        LOG_D("%d, ",reqs_since_last_log);
                        LOG_I("%d\n",capacity_change);
                    } else if (reqs_since_last_log > REQS_PER_LOG) {
                        curr_capacity = present_power();
                        capacity_change = last_capacity - curr_capacity;
                        last_capacity = curr_capacity;
                        LOG_D("%d\n",capacity_change);
                        LOG_D("%d, ",reqs_since_last_log);
                        reqs_since_last_log = 1;
                    }
                #endif
                ret = serve_write(&resp, sub_resp_buf, tenant, request->write);
                break;
	    case SUNNEED_REQUEST__MESSAGE_TYPE_SOCKET:
		ret = serve_socket(&resp, sub_resp_buf, request->socket);
		break;
	    case SUNNEED_REQUEST__MESSAGE_TYPE_CONNECT:
		ret = serve_connect(pipe, request->connect);
		break;
	    case SUNNEED_REQUEST__MESSAGE_TYPE_SEND:
		ret = serve_send(tenant, request->send);
		break;
            default:
                LOG_W("Received request with invalid type %d", request->message_type_case);
                ret = -1;
                #ifdef LOG_PWR
                    reqs_since_last_log--;
                #endif
                break;
        }

        resp.status = ret;

        // Create and send the response message.
        nng_msg *resp_msg;
        int resp_len = sunneed_response__get_packed_size(&resp);
        void *resp_buf = malloc(resp_len);
        sunneed_response__pack(&resp, resp_buf);

        SUNNEED_NNG_TRY(nng_msg_alloc, != 0, &resp_msg, resp_len);
        SUNNEED_NNG_TRY(nng_msg_insert, != 0, resp_msg, resp_buf, resp_len);
        SUNNEED_NNG_TRY(nng_sendmsg, != 0, sock, resp_msg, 0);

    end:
        sunneed_request__free_unpacked(request, NULL);
        nng_msg_free(resp_msg);
        nng_msg_free(msg);
    }

    free(sub_resp_buf);
}
