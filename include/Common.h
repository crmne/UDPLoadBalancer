#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>

#define MAXCONNECTIONS 1
#define MAXPATHS 3

typedef struct
{
	uint32_t n;
	uint16_t port[MAXPATHS];
} config_t;

void configSigHandlers();

int listenFromApp(int);
int acceptFromApp(int);
int connectToMon(int);
char recvMonitorPkts(int, config_t *);

#endif
