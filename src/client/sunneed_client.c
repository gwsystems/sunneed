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

    SUNNEED_NNG_TRY(nng_msg_alloc, != 0, &msg, req_len);        
    SUNNEED_NNG_TRY(nng_msg_insert, != 0, msg, buf, req_len);   
    SUNNEED_NNG_TRY(nng_sendmsg, != 0, sunneed_socket, msg, 0);

    free(buf);
    nng_msg_free(msg);                                         
}

static SunneedResponse *
receive_response(SunneedResponse__MessageTypeCase message_type) {
    nng_msg *reply;
    SUNNEED_NNG_TRY(nng_recvmsg, != 0, sunneed_socket, &reply, 0);

    size_t msg_len = nng_msg_len(reply);
    SUNNEED_NNG_MSG_LEN_FIX(msg_len);
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

    
    if(!(client_init))
    {
	    client_init = 1;
    }    
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

	if(!((domain == AF_INET) || (domain == AF_INET6)))
	{
		perror("invalid address family, must be ipv4 or ipv6\n");
		exit(0);
	}

	if(!((type == 1) || (type == 2)))
	{
		perror("invalid type, must be SOCK_STREAM(tcp) or SOCK_DGRAM(udp)\n");
		exit(0);
	}

	SunneedRequest req = SUNNEED_REQUEST__INIT;
	req.message_type_case = SUNNEED_REQUEST__MESSAGE_TYPE_SOCKET;

	SocketRequest sock = SOCKET_REQUEST__INIT;
	sock.domain = domain;
	sock.type = type;

	//TODO: check protocol
	sock.protocol = protocol;

	req.socket = &sock;
	send_request(&req);

	SunneedResponse *resp = receive_response(SUNNEED_RESPONSE__MESSAGE_TYPE_SOCKET);
	if(resp == NULL)
	{
		FATAL(-1, "failed to create socket");
	}

	int i;
	for(i = 0; i < MAX_TENANT_SOCKETS; i++)
	{
		if(dummy_sockets[i].dummy_sockfd == -1)
		{
			dummy_sockets[i].dummy_sockfd = resp -> socket -> dummy_sockfd;
			sunneed_response__free_unpacked(resp, NULL);
			printf("added dummy sockfd to table\n");
			return dummy_sockets[i].dummy_sockfd;
		}
	}

	//TODO: handle not having enough free sockets
	return -1;

}

int
sunneed_client_is_dummysocket(int sockfd)
{
	int i;
	for(i = 0; i < MAX_TENANT_SOCKETS; i++)
	{
		if(dummy_sockets[i].dummy_sockfd == sockfd)
		{
			printf("found dummy sockfd\n");
			return 1;
		}
	}
	return 0;
}

int
sunneed_client_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	printf("SUNNEED CLIENT CONNECT\n\n");
	char address[NI_MAXHOST];
	int port = 0;
	printf("get name info was here\n");
	//getnameinfo(addr, addrlen, address, sizeof(address), NULL, 0, 0);
	//printf("client connect: got address %s\n", address);

	SunneedRequest req = SUNNEED_REQUEST__INIT;
	req.message_type_case = SUNNEED_REQUEST__MESSAGE_TYPE_CONNECT;

	ConnectRequest conn = CONNECT_REQUEST__INIT;
	conn.port = port;
	conn.address = address;
	conn.addrlen = (int)addrlen;

	req.connect = &conn;
	send_request(&req);

	return 0;
}

ssize_t 
sunneed_client_remote_send(int sockfd, const void *data, size_t len, int flags)
{
	//TODO: check sockfd for real socket, check data, flags, etc
	//for now, just tell sunneed to perform a send for us
	
	if(!sockfd)
	{
		perror("bad socket fd");
		exit(0);
	}

	SunneedRequest req = SUNNEED_REQUEST__INIT;
	req.message_type_case = SUNNEED_REQUEST__MESSAGE_TYPE_SEND;

	SendRequest send_req = SEND_REQUEST__INIT;
	send_req.sockfd = sockfd;
	send_req.data.data = malloc(len);

	memcpy(send_req.data.data, data, len);
	send_req.data.len = len;

	req.send = &send_req;
	send_request(&req);

	free(send_req.data.data);

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
