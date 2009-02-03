#include <string.h>
#include <err.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "macro.h"

struct sockaddr_in set_sock(const char *addr, int port)
{
    struct sockaddr_in sock;

    memset(&sock, 0, sizeof(sock));
    sock.sin_family = AF_INET;
    if (inet_aton(addr, &sock.sin_addr) == 0)
	err(1, "set_sock(addr=%s,port=%d): inet_aton()", addr, port);
    sock.sin_port = htons(port);
    return sock;
}

int create_sock(int type)
{
    int on;
    int socketfd = socket(AF_INET, type, 0);

    if (socketfd < 0)
	err(1, "create_sock(type=%d): socket()", type);

    /* avoid EADDRINUSE */
    on = 1;
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) <
	0)
	err(1, "create_sock(type=%d): setsockopt(socketfd=%d)", type,
	    socketfd);

    return socketfd;
}

/** Sets up a listening socket
 * @param port which we want to listen
 * @return file descriptor of created socket
 */
int listen_app(const char *addr, int port)
{
    struct sockaddr_in sock;
    int socketfd;

    socketfd = create_sock(SOCK_STREAM);
    sock = set_sock(addr, port);
    if (bind(socketfd, (struct sockaddr *) &sock, sizeof(sock)) < 0)
	err(1, "listen_app(port=%d): bind(socketfd=%d)", port, socketfd);
    if (listen(socketfd, MAX_CONNS) < 0)
	err(1, "listen_app(port=%d): listen(socketfd=%d)", port, socketfd);
#ifdef DEBUG
    printf("Waiting for App on port %d...\n", port);
    fflush(stdout);
#endif
    return socketfd;
}

int connect_mon(const char *addr, int port)
{
    struct sockaddr_in local, serv;
    int socketfd;

    socketfd = create_sock(SOCK_STREAM);
    local = set_sock(addr, 0);
    if (bind(socketfd, (struct sockaddr *) &local, sizeof(local)) < 0)
	err(1, "connect_mon(port=%d): bind(socketfd=%d)", port, socketfd);

    serv = set_sock(addr, port);
    if (connect(socketfd, (struct sockaddr *) &serv, sizeof(serv)) < 0)
	err(1, "connect_mon(port=%d): connect(socketfd=%d)", port,
	    socketfd);
#ifdef DEBUG
    printf("Connected to Monitor on port %d\n", port);
    fflush(stdout);
#endif
    return socketfd;
}

int accept_app(int socketfd)
{
    int newsocketfd;
    unsigned int len;
    struct sockaddr_in sock;

    len = sizeof(sock);
    memset(&sock, 0, len);
    newsocketfd = accept(socketfd, (struct sockaddr *) &sock, &len);
    if (newsocketfd < 0)
	err(1, "accept_app(socketfd=%d): accept()", socketfd);
#ifdef DEBUG
    printf("Connected to App\n");
    fflush(stdout);
#endif
    return newsocketfd;

}


int listen_udp(const char *addr, int port)
{
    struct sockaddr_in addr_in;
    int socketfd = create_sock(SOCK_DGRAM);

    addr_in = set_sock(addr, port);
    if (bind(socketfd, (struct sockaddr *) &addr_in, sizeof(addr_in)) < 0)
	err(1, "listen_app(port=%d): bind(socketfd=%d)", port, socketfd);
    return socketfd;
}

int connect_udp(const char *addr, int port)
{
    struct sockaddr_in addr_in;
    int socketfd = create_sock(SOCK_DGRAM);

    addr_in = set_sock(addr, port);
    if (connect(socketfd, (struct sockaddr *) &addr_in, sizeof(addr_in)) <
	0)
	err(1, "connect_udp(): connect(port=%d,socketfd=%d)", port,
	    socketfd);
    return socketfd;
}
