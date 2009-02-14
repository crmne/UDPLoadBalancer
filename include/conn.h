#ifndef __CONN_H__
#define __CONN_H__

#include <netinet/in.h>

int create_sock(int);
struct sockaddr_in set_sock(const char *, int);
int listen_app(const char *, int);
int accept_app(int);
int connect_mon(const char *, int);
int listen_udp(const char *, int);
int connect_udp(const char *, int);

#endif
