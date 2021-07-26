#include "sunneed_overlay.h"

void
on_load() {

    sunneed_client_init("TODO");
    printf("Overlay: Client init\n");
    
}

void
on_unload() {
    sunneed_client_disconnect();
}

int
open(const char *pathname, int flags, mode_t mode) {
    printf("Opening file %s\n", pathname);

    int locked = sunneed_client_check_locked_file(pathname);
    if (locked < 0) {
        printf("'%s' is not locked; opening normally\n", pathname);
    } else {
        printf("'%s' is locked; opening via dummy\n", pathname);
        char *dummy_path = sunneed_client_fetch_locked_file_path(pathname, flags);
        pathname = dummy_path;
    }

    int fd;
    SUPER(fd, open, int, (pathname, flags, mode), const char *, int, mode_t);

    // TODO Handle errors from open

    if(locked > 0) sunneed_client_on_locked_path_open(locked, (char *)pathname, fd);

    sunneed_client_debug_print_locked_path_table();
    

    return fd;
}

ssize_t
write(int fd, const void *buf, size_t count) {
    printf("Overlay write %d\n", fd);
    int ret;

    if (!sunneed_client_fd_is_locked(fd)) {
        // Perform the write as normal.
        SUPER(ret, write, int, (fd, buf, count), int, const void *, size_t);
        return ret;
    }
    
    // Ask sunneed to do the write for us.
    sunneed_client_remote_write(fd, buf, count);

    return 0;
}

int
socket(int domain, int type, int protocol)
{
	int ret;

	ret = sunneed_client_socket(domain, type, protocol);
		
	if(ret == -1)
	{
		//nng socket is not open (caught in sunneed_client_socket), call SUPER
		SUPER(ret, socket, int, (domain, type, protocol), int, int, int);
		return ret;
	}

	return ret;

}

int
connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	printf("overlay connect\n");

	int ret;

	if(sunneed_client_is_dummysocket(sockfd))
	{ 
		ret = sunneed_client_connect(sockfd, addr, addrlen);
		if(ret == -1)
		{
			//NNG dialer wasn't set up or the socket type was wrong
			SUPER(ret, connect, int, (sockfd, addr, addrlen), int, const struct sockaddr *, socklen_t);
			return ret;
		}

		return ret;
	}else if(sockfd){
		//non-sunneed socket (maybe dont need this? Need to think of a case where we'd want to allow a connection to a non sunneed socket)
		SUPER(ret, connect, int, (sockfd, addr, addrlen), int, const struct sockaddr *, socklen_t);
		return ret;
	}else{
		perror("overlay connect: bad socketfd and not a dummy socket\n");
		return 0;
	}

}

ssize_t
send(int sockfd, const void *buf, size_t len, int flags)
{
	if(sunneed_client_is_dummysocket(sockfd))
	{
		return sunneed_client_remote_send(sockfd, buf, len, flags);
	}else if(sockfd){
		int ret;
		SUPER(ret, send, int, (sockfd, buf, len, flags), int, const void *, size_t, int);
		return ret;
	}
	return 0;
}




