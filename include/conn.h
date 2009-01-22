#ifndef __CONN_H__
#define __CONN_H__

#include <netinet/in.h>

int createSocket4(int);
struct sockaddr_in setSocket4(const char *, int);
int listenFromApp(const char *, int);
int acceptFromApp(int);
int connectToMon(const char *, int);
int listenUDP4(const char *, int);
int connectUDP4(const char *, int);

#endif
