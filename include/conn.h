#ifndef __CONN_H__
#define __CONN_H__

#include <netinet/in.h>

int createSocket4(int);
struct sockaddr_in setSocket4(const char *, int);
int listenFromApp(int);
int acceptFromApp(int);
int connectToMon(int);
int listenUDP4(int);

#endif
