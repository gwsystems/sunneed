<<<<<<< HEAD
#include "sunneed_client.h"
#include "../log.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

struct {
    char *path;
    int fd;
} locked_paths[MAX_LOCKED_FILES] = { { NULL, 0 } };

struct {
	int dummy_sockfd;
} dummy_sockets[MAX_TENANT_SOCKETS] = { { -1 } };

nng_socket sunneed_socket;
nng_dialer sunneed_socket_dialer;
nng_msg *msg;
static void
nngfatal(const char *func, int rv) {
    FATAL(rv, "%s: %s\n", func, nng_strerror(rv));
}

/** 
 * Packs the given SunneedRequest into an NNG message and sends it to sunneed. If any failures occur in the packing
 *  or sending processes, this client will crash with a fatal error.
 */
static void
send_request(SunneedRequest *req) {
    //nng_msg *msg;                                               
    int req_len = sunneed_request__get_packed_size(req);
    void *buf = malloc(req_len);
    if (!buf)
        FATAL(-1, "unable to allocate buffer for request");
    sunneed_request__pack(req, buf);                           
    int ret = -1;
    SUNNEED_NNG_TRY(nng_msg_alloc, != 0, &msg, 0);  
    SUNNEED_NNG_TRY(nng_msg_append,  != 0, msg, buf, req_len);
    SUNNEED_NNG_TRY(nng_sendmsg, != 0, sunneed_socket, msg, 0);

      
    free(buf); 
}

static SunneedResponse *
receive_response(SunneedResponse__MessageTypeCase message_type) {
    nng_msg *reply;
    SUNNEED_NNG_TRY(nng_recvmsg, != 0, sunneed_socket, &reply, 0);

    size_t msg_len = nng_msg_len(reply);
    //SUNNEED_NNG_MSG_LEN_FIX(msg_len);
    SunneedResponse *resp = sunneed_response__unpack(NULL, msg_len, nng_msg_body(reply));

    if (resp->status != 0) {
        return NULL;
    } else if (resp->message_type_case != message_type) {
        FATAL(-1, "incorrect message type received (expected %d, got %d)", message_type, resp->status);
    }

    nng_msg_free(reply);
    return resp;
}

int
sunneed_client_init(const char *name) {
   
    SUNNEED_NNG_SET_ERROR_REPORT_FUNC(nngfatal);
    SUNNEED_NNG_TRY(nng_req0_open, != 0, &sunneed_socket);
    SUNNEED_NNG_TRY(nng_dial, != 0, sunneed_socket, SUNNEED_LISTENER_URL, &sunneed_socket_dialer, 0);
 

    SunneedRequest req = SUNNEED_REQUEST__INIT;
    req.message_type_case = SUNNEED_REQUEST__MESSAGE_TYPE_REGISTER_CLIENT;
    RegisterClientRequest register_req = REGISTER_CLIENT_REQUEST__INIT;
    register_req.name = malloc(strlen(name) + 1);
    if (!register_req.name) {
        FATAL(-1, "failed to allocate memory for client name");
        return -1;
    }
    strncpy(register_req.name, name, strlen(name) + 1);
    req.register_client = &register_req;
    send_request(&req);
    free(register_req.name);

    // Check the response.
    SunneedResponse *resp = receive_response(SUNNEED_RESPONSE__MESSAGE_TYPE_REGISTER_CLIENT);
    for (size_t i = 0; i < resp->register_client->n_locked_paths; i++) {
        locked_paths[i].path = malloc(strlen(resp->register_client->locked_paths[i]) + 1);
        strcpy(locked_paths[i].path, resp->register_client->locked_paths[i]);
        client_printf(-1, "Registered locked path '%s'", locked_paths[i].path);
    }
    sunneed_response__free_unpacked(resp, NULL);

    return 0;
}

/** Allocate a string containing the path of the dummy file corresponding to the given path. */
char *
sunneed_client_fetch_locked_file_path(const char *pathname, int flags) {
    // TODO Check socket opened.

    SunneedRequest req = SUNNEED_REQUEST__INIT;
    req.message_type_case = SUNNEED_REQUEST__MESSAGE_TYPE_OPEN_FILE;
    OpenFileRequest open_file_req = OPEN_FILE_REQUEST__INIT;
    open_file_req.path = malloc(strlen(pathname) + 1);
    if (!open_file_req.path) {
        FATAL(-1, "failed to allocated memory for path");
    }
    strncpy(open_file_req.path, pathname, strlen(pathname) + 1);

    open_file_req.flags = flags;

    req.open_file = &open_file_req;

    send_request(&req);
    free(open_file_req.path);

    // TODO Handle request of a path that isn't locked.
    SunneedResponse *resp = receive_response(SUNNEED_RESPONSE__MESSAGE_TYPE_OPEN_FILE);
    if (resp == NULL) {
        // TODO Gotos
        return 0;
    }

    client_printf("Opening dummy path '%s'\n", resp->open_file->path);
    char *path = malloc(strlen(resp->open_file->path) + 1);
    if (!path)
        FATAL(-1, "unable to allocate path");
    strcpy(path, resp->open_file->path);

    sunneed_response__free_unpacked(resp, NULL);

    return path;
}

int
sunneed_client_check_locked_file(const char *pathname) {
    for (int i = 0; i < MAX_LOCKED_FILES; i++) {
        if (locked_paths[i].path == NULL)
            return -1;
        else if (strncmp(pathname, locked_paths[i].path, strlen(pathname)) == 0)
            return i; 
    }

    return -1;
}

int
sunneed_client_on_locked_path_open(int i, char *pathname, int fd) {
    if (i < 0 || i >= MAX_LOCKED_FILES)
        FATAL(-1, "locked-file array index out of bounds");
    if (pathname == NULL)
        FATAL(-1, "pathname is null");
    if (fd <= 0)
        FATAL(-1, "illegal FD");

	   //toSend[REQUETS_PER_PWR_LOG * 3] = pwr_change;
    locked_paths[i].path = pathname;
    locked_paths[i].fd = fd;

    return 0;
}

bool
sunneed_client_fd_is_locked(int fd) {
    for (int i = 0; i < MAX_LOCKED_FILES; i++)
        if (locked_paths[i].fd == fd)
            return true;
    return false;
}

ssize_t
sunneed_client_remote_write(int fd, const void *data, size_t n_bytes) {
    // Get the dummy path corresponding to the FD.
    int locked_file_i;
    char *dummy_path = NULL;
    for (locked_file_i = 0; locked_file_i < MAX_LOCKED_FILES; locked_file_i++)
        if (locked_paths[locked_file_i].fd == fd) {
            dummy_path = locked_paths[locked_file_i].path;
            break;
        }

    if (dummy_path == NULL)
        FATAL(-1, "cannot remote write a non-dummy file");

    SunneedRequest req = SUNNEED_REQUEST__INIT;
    req.message_type_case = SUNNEED_REQUEST__MESSAGE_TYPE_WRITE;

    WriteRequest write_req = WRITE_REQUEST__INIT;
    write_req.dummy_path = malloc(strlen(dummy_path) + 1);
    write_req.data.data = malloc(n_bytes);
    if (!write_req.dummy_path || !write_req.data.data)
        FATAL(-1, "failed to allocate write request data");
    strncpy((char *)write_req.data.data, data, n_bytes);
    strncpy(write_req.dummy_path, dummy_path, strlen(dummy_path) + 1);
    write_req.data.len = n_bytes;

    req.write = &write_req;
    send_request(&req);

    free(write_req.dummy_path);
    free(write_req.data.data);

    SunneedResponse *resp = receive_response(SUNNEED_RESPONSE__MESSAGE_TYPE_CALL_WRITE);
    if (resp == NULL) {
        // TODO Handle
        FATAL(-1, "write response was NULL");
    }
    sunneed_response__free_unpacked(resp, NULL);

    return 0;
}

int
sunneed_client_socket(int domain, int type, int protocol)
{

    /*
     * sunneed_socket_dialer points to the nng_dialer associated with the tenant <--> suneeed nng socket
     * since the dialer is the last part of the socket set up, this should indicate whether the nng socket is set up or not
     * if not, nng_dialer_id will return -1 to signal an invalid dialer, therefore we need to call SUPER to create the nng socket
     */
    if(nng_dialer_id(sunneed_socket_dialer) == -1)
    {
        return -1;
    }

    /*
     * currently only support IPv4 UDP packets for power logging
     * can extend for TCP and IPv6 later if needed
     */
	if(domain != AF_INET)
	{
		FATAL(-1, "must be IPv4 socket\n");
	}

	if(type != SOCK_DGRAM)
	{
        //TODO: determine type of getnameinfo and getaddrinfo packets, both end up triggering this return case
        //should error out if not one of those, but for now just return -1 to signal a call to SUPER
		return -1;
	}

	SunneedRequest req = SUNNEED_REQUEST__INIT;
	req.message_type_case = SUNNEED_REQUEST__MESSAGE_TYPE_SOCKET;

	SocketRequest sock = SOCKET_REQUEST__INIT;
	sock.domain = domain;
	sock.type = type;


	sock.protocol = protocol;

	req.socket = &sock;
	send_request(&req);

	SunneedResponse *resp = receive_response(SUNNEED_RESPONSE__MESSAGE_TYPE_SOCKET);
	if(resp == NULL)
	{
		FATAL(-1, "failed to create socket, no response");
	}

    //add the new fake socket from sunneed to the table, future sends to this fd will be caught by the send overlay
	int i;
	for(i = 0; i < MAX_TENANT_SOCKETS; i++)
	{
		if(dummy_sockets[i].dummy_sockfd == -1)
		{
			dummy_sockets[i].dummy_sockfd = resp -> socket -> dummy_sockfd;
			sunneed_response__free_unpacked(resp, NULL);
			return dummy_sockets[i].dummy_sockfd;
		}
	}

	//TODO: handle not having enough free sockets
	sunneed_response__free_unpacked(resp, NULL);
	return -1;

}

/*
 * dummy socket table lookup function for convenience
 * TODO: could probably speed this up a little bit (currently O(n))
 */
int
sunneed_client_is_dummysocket(int sockfd)
{
	int i;
	for(i = 0; i < MAX_TENANT_SOCKETS; i++)
	{
		if(dummy_sockets[i].dummy_sockfd == sockfd)
		{
			return 1;
		}
	}
	return 0;
}

int
sunneed_client_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    
	char host_name[NI_MAXHOST];
	char address[INET_ADDRSTRLEN];
	int port = 0;
	struct hostent *requested_host;
    struct sockaddr_in *addr_info;
    struct addrinfo hints;
    struct addrinfo *result, *rp;
	void *addr_ptr;

    //need to check that nng is set up via dialer here as well
    if(nng_dialer_id(sunneed_socket_dialer) == -1)
    {
        return -1;
    }

    
	/*
     * get remote address and port from given sockaddr struct
     * uses AF_INET since we assume IPv4 packets, change to AF_UNSPEC when IPv6 support is added
     */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

	getnameinfo(addr, addrlen, host_name, NI_MAXHOST, NULL, 0, 0);
    if(getaddrinfo(host_name, NULL, &hints, &result))
    {
        FATAL(-1, "getaddrinfo\n");
    }

    //getaddrinfo() returns a list of socket structs, since we only have one socket per address this will give us the address the tenant requested
    while(result)
    {
        inet_ntop(result->ai_family, result->ai_addr->sa_data, address, NI_MAXHOST);  
        addr_ptr = &((struct sockaddr_in*)result->ai_addr)->sin_addr;

        inet_ntop(result->ai_family, addr_ptr, address, NI_MAXHOST);

        
        result = result->ai_next;
    }

    addr_info = (struct sockaddr_in*) addr;
    port = htons(addr_info->sin_port);

	SunneedRequest req = SUNNEED_REQUEST__INIT;
	req.message_type_case = SUNNEED_REQUEST__MESSAGE_TYPE_CONNECT;

	ConnectRequest conn = CONNECT_REQUEST__INIT;
	conn.port = port;
	conn.address = address;
	conn.addrlen = sizeof(address);
    conn.sockfd = sockfd;

	req.connect = &conn;
	send_request(&req);

    SunneedResponse *resp = receive_response(SUNNEED_RESPONSE__MESSAGE_TYPE_GENERIC);
    if(resp == NULL)
    {
        FATAL(-1, "connect response was null\n");
        return -1;
    }

    sunneed_response__free_unpacked(resp, NULL);

    return 1;
}

ssize_t 
sunneed_client_remote_send(int sockfd, const void *data, size_t len, int flags)
{
	
	SunneedRequest req = SUNNEED_REQUEST__INIT;
	req.message_type_case = SUNNEED_REQUEST__MESSAGE_TYPE_SEND;

	SendRequest send_req = SEND_REQUEST__INIT;
	send_req.sockfd = sockfd;
	send_req.data.data = malloc(len);

    if(!(send_req.data.data))
    {
        printf("failed to malloc request data\n");
        return -1;
    }

	strncpy(send_req.data.data, data, len);
	send_req.data.len = len;

	req.send = &send_req;
	send_request(&req);

    SunneedResponse *resp = receive_response(SUNNEED_RESPONSE__MESSAGE_TYPE_GENERIC);
    if(resp == NULL)
    {
        FATAL(-1, "send response was null\n");
        return -1;
    }
    free(send_req.data.data);
    sunneed_response__free_unpacked(resp, NULL);

    return 0;
}

int
sunneed_client_disconnect(void) {
    // TODO Check socket opened.

    SunneedRequest req = SUNNEED_REQUEST__INIT;
    req.message_type_case = SUNNEED_REQUEST__MESSAGE_TYPE_UNREGISTER_CLIENT;
    UnregisterClientRequest unregister_req = UNREGISTER_CLIENT_REQUEST__INIT;
    req.unregister_client = &unregister_req;
    send_request(&req);

    SunneedResponse *resp = receive_response(SUNNEED_RESPONSE__MESSAGE_TYPE_GENERIC);
    if (resp == NULL) {
        // TODO Handle
        FATAL(-1, "Disconnect response was NULL\n");
    }
    sunneed_response__free_unpacked(resp, NULL);

    client_printf("Unregistered.\n");
    return 0;
}

void
sunneed_client_debug_print_locked_path_table(void) {
    client_printf("locked files: [\n");
    for (int i = 0; i < MAX_LOCKED_FILES; i++) {
        if (locked_paths[i].path != NULL)
            client_printf("    FD %d : '%s'\n", locked_paths[i].fd, locked_paths[i].path);
    }
    client_printf("]\n");
}
=======
#include "sunneed_client.h"
#include "../log.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

struct {
    char *path;
    int fd;
} locked_paths[MAX_LOCKED_FILES] = { { NULL, 0 } };

nng_socket sunneed_socket;

static void
nngfatal(const char *func, int rv) {
    FATAL(rv, "%s: %s\n", func, nng_strerror(rv));
}

/** 
 * Packs the given SunneedRequest into an NNG message and sends it to sunneed. If any failures occur in the packing
 *  or sending processes, this client will crash with a fatal error.
 */
static void
send_request(SunneedRequest *req) {
    nng_msg *msg;                                               

    int req_len = sunneed_request__get_packed_size(req);
    void *buf = malloc(req_len);
    if (!buf)
        FATAL(-1, "unable to allocate buffer for request");
    sunneed_request__pack(req, buf);                           

//    SUNNEED_NNG_TRY(nng_msg_alloc, != 0, &msg, req_len);        
//    SUNNEED_NNG_TRY(nng_msg_insert, != 0, msg, buf, req_len);   
    SUNNEED_NNG_TRY(nng_msg_alloc, != 0, &msg, 0);
    SUNNEED_NNG_TRY(nng_msg_append, != 0, msg, buf, req_len);
    SUNNEED_NNG_TRY(nng_sendmsg, != 0, sunneed_socket, msg, 0);

    free(buf);
//    nng_msg_free(msg);                                         
}

static SunneedResponse *
receive_response(SunneedResponse__MessageTypeCase message_type) {
    nng_msg *reply;
    SUNNEED_NNG_TRY(nng_recvmsg, != 0, sunneed_socket, &reply, 0);

    size_t msg_len = nng_msg_len(reply);
//    SUNNEED_NNG_MSG_LEN_FIX(msg_len);
    SunneedResponse *resp = sunneed_response__unpack(NULL, msg_len, nng_msg_body(reply));

    if (resp->status != 0) {
        return NULL;
    } else if (resp->message_type_case != message_type) {
        FATAL(-1, "incorrect message type received (expected %d, got %d)", message_type, resp->status);
    }

    nng_msg_free(reply);
    return resp;
}

int
sunneed_client_init(const char *name) {
    SUNNEED_NNG_SET_ERROR_REPORT_FUNC(nngfatal);
    SUNNEED_NNG_TRY(nng_req0_open, != 0, &sunneed_socket);
    SUNNEED_NNG_TRY(nng_dial, != 0, sunneed_socket, SUNNEED_LISTENER_URL, NULL, 0);

    // Register this client with sunneed.
    SunneedRequest req = SUNNEED_REQUEST__INIT;
    req.message_type_case = SUNNEED_REQUEST__MESSAGE_TYPE_REGISTER_CLIENT;
    RegisterClientRequest register_req = REGISTER_CLIENT_REQUEST__INIT;
    register_req.name = malloc(strlen(name) + 1);
    if (!register_req.name) {
        FATAL(-1, "failed to allocate memory for client name");
        return -1;
    }
    strncpy(register_req.name, name, strlen(name) + 1);
    req.register_client = &register_req;
    send_request(&req);
    free(register_req.name);

    // Check the response.
    SunneedResponse *resp = receive_response(SUNNEED_RESPONSE__MESSAGE_TYPE_REGISTER_CLIENT);
    for (size_t i = 0; i < resp->register_client->n_locked_paths; i++) {
        locked_paths[i].path = malloc(strlen(resp->register_client->locked_paths[i]) + 1);
        strcpy(locked_paths[i].path, resp->register_client->locked_paths[i]);
        client_printf(-1, "Registered locked path '%s'", locked_paths[i].path);
    }
    sunneed_response__free_unpacked(resp, NULL);

    return 0;
}

/** Allocate a string containing the path of the dummy file corresponding to the given path. */
char *

sunneed_client_fetch_locked_file_path(const char *pathname, int flags, int mode) {
    // TODO Check socket opened.

    SunneedRequest req = SUNNEED_REQUEST__INIT;
    req.message_type_case = SUNNEED_REQUEST__MESSAGE_TYPE_OPEN_FILE;
    OpenFileRequest open_file_req = OPEN_FILE_REQUEST__INIT;
    open_file_req.path = malloc(strlen(pathname) + 1);
    if (!open_file_req.path) {
        FATAL(-1, "failed to allocated memory for path");
    }
    strncpy(open_file_req.path, pathname, strlen(pathname) + 1);

    open_file_req.flags = flags;

    req.open_file = &open_file_req;

    open_file_req.flags = flags;
    open_file_req.mode = mode;


    send_request(&req);
    free(open_file_req.path);

    // TODO Handle request of a path that isn't locked.
    SunneedResponse *resp = receive_response(SUNNEED_RESPONSE__MESSAGE_TYPE_OPEN_FILE);
    if (resp == NULL) {
        // TODO Gotos
        return 0;
    }

    client_printf("Opening dummy path '%s'\n", resp->open_file->path);
    char *path = malloc(strlen(resp->open_file->path) + 1);
    if (!path)
        FATAL(-1, "unable to allocate path");
    strcpy(path, resp->open_file->path);

    sunneed_response__free_unpacked(resp, NULL);

    return path;
}

int
sunneed_client_check_locked_file(const char *pathname) {
    for (int i = 0; i < MAX_LOCKED_FILES; i++) {
        if (locked_paths[i].path == NULL)
            return -1;
        else if (strncmp(pathname, locked_paths[i].path, strlen(pathname)) == 0)
            return i; 
    }

    return -1;
}

int
sunneed_client_on_locked_path_open(int i, char *pathname, int fd) {
    if (i < 0 || i >= MAX_LOCKED_FILES)
        FATAL(-1, "locked-file array index out of bounds");
    if (pathname == NULL)
        FATAL(-1, "pathname is null");
    if (fd <= 0)
	FATAL(-1, "illegal FD");

    locked_paths[i].path = pathname;
    locked_paths[i].fd = fd;

    return 0;
}

bool
sunneed_client_fd_is_locked(int fd) {
    for (int i = 0; i < MAX_LOCKED_FILES; i++)
        if (locked_paths[i].fd == fd)
            return true;
    return false;
}

ssize_t
sunneed_client_remote_write(int fd, const void *data, size_t n_bytes) {
    // Get the dummy path corresponding to the FD.
    int locked_file_i;
    char *dummy_path = NULL;
    for (locked_file_i = 0; locked_file_i < MAX_LOCKED_FILES; locked_file_i++)
        if (locked_paths[locked_file_i].fd == fd) {
            dummy_path = locked_paths[locked_file_i].path;
            break;
        }

    if (dummy_path == NULL)
        FATAL(-1, "cannot remote write a non-dummy file");

    SunneedRequest req = SUNNEED_REQUEST__INIT;
    req.message_type_case = SUNNEED_REQUEST__MESSAGE_TYPE_WRITE;

    WriteRequest write_req = WRITE_REQUEST__INIT;
    write_req.dummy_path = malloc(strlen(dummy_path) + 1);
    write_req.data.data = malloc(n_bytes);
    if (!write_req.dummy_path || !write_req.data.data)
        FATAL(-1, "failed to allocate write request data");
    strncpy((char *)write_req.data.data, data, n_bytes);
    strncpy(write_req.dummy_path, dummy_path, strlen(dummy_path) + 1);
    write_req.data.len = n_bytes;

    req.write = &write_req;
    send_request(&req);

    free(write_req.dummy_path);
    free(write_req.data.data);

    SunneedResponse *resp = receive_response(SUNNEED_RESPONSE__MESSAGE_TYPE_CALL_WRITE);
    if (resp == NULL) {
        // TODO Handle
        FATAL(-1, "write response was NULL");
    }
    sunneed_response__free_unpacked(resp, NULL);

    return 0;
}

int
sunneed_client_disconnect(void) {
    // TODO Check socket opened.

    SunneedRequest req = SUNNEED_REQUEST__INIT;
    req.message_type_case = SUNNEED_REQUEST__MESSAGE_TYPE_UNREGISTER_CLIENT;
    UnregisterClientRequest unregister_req = UNREGISTER_CLIENT_REQUEST__INIT;
    req.unregister_client = &unregister_req;
    send_request(&req);

    SunneedResponse *resp = receive_response(SUNNEED_RESPONSE__MESSAGE_TYPE_GENERIC);
    if (resp == NULL) {
        // TODO Handle
        FATAL(-1, "Disconnect response was NULL\n");
    }
    sunneed_response__free_unpacked(resp, NULL);

    client_printf("Unregistered.\n");
    return 0;
}

void
sunneed_client_debug_print_locked_path_table(void) {
    client_printf("locked files: [\n");
    for (int i = 0; i < MAX_LOCKED_FILES; i++) {
        if (locked_paths[i].path != NULL)
            client_printf("    FD %d : '%s'\n", locked_paths[i].fd, locked_paths[i].path);
    }
    client_printf("]\n");
}
>>>>>>> c1ce9be87ee40f52cd027a965adce8ad5d136a36
