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
        err(ERR_INETATON);
    sock.sin_port = htons(port);
    return sock;
}

int create_sock(int type)
{
    int on;
    int socketfd = socket(AF_INET, type, 0);

    if (socketfd < 0)
        err(ERR_SOCKET);

    /* avoid EADDRINUSE */
    on = 1;
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) <
        0)
        err(ERR_SETSOCKOPT);

    return socketfd;
}

int listen_app(const char *addr, int port)
{
    struct sockaddr_in sock;
    int socketfd;

    socketfd = create_sock(SOCK_STREAM);
    sock = set_sock(addr, port);
    if (bind(socketfd, (struct sockaddr *) &sock, sizeof(sock)) < 0)
        err(ERR_BIND);
    if (listen(socketfd, MAX_CONNS) < 0)
        err(ERR_LISTEN);
    printf("Waiting for App to connect on port %d... ", port);
    fflush(stdout);
    return socketfd;
}

int connect_mon(const char *addr, int port)
{
    struct sockaddr_in local, serv;
    int socketfd;

    socketfd = create_sock(SOCK_STREAM);
    local = set_sock(addr, 0);
    if (bind(socketfd, (struct sockaddr *) &local, sizeof(local)) < 0)
        err(ERR_BIND);

    serv = set_sock(addr, port);
    if (connect(socketfd, (struct sockaddr *) &serv, sizeof(serv)) < 0)
        err(ERR_CONNECT);
    printf("Connected to Monitor on port %d\n", port);
    fflush(stdout);
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
        err(ERR_ACCEPT);
    printf("connected!\n");
    fflush(stdout);
    return newsocketfd;

}


int bind_udp(const char *addr, int port)
{
    struct sockaddr_in addr_in;
    int socketfd = create_sock(SOCK_DGRAM);

    addr_in = set_sock(addr, port);
    if (bind(socketfd, (struct sockaddr *) &addr_in, sizeof(addr_in)) < 0)
        err(ERR_BIND);
    return socketfd;
}
