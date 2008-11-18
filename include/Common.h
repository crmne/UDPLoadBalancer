#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>

#define MAXCONNECTIONS 1
#define MAXPATHS 3
#define PKTSIZE 100

typedef struct
{
	char type;
	uint32_t n;
	uint16_t port[MAXPATHS];
	int socket[MAXPATHS];
} config_t;

typedef struct
{
	uint32_t id;
	struct timeval time;
	char data[PKTSIZE - sizeof(uint32_t) - sizeof(struct timeval)];
} packet_t;

void configSigHandlers();

int listenFromApp(int);
int acceptFromApp(int);
int connectToMon(int);
char recvMonitorPkts(int, config_t *);
uint32_t recvVoicePkts(int, packet_t *);
void sendVoicePkts(int, packet_t *);
void reconfigRoutes(config_t *, config_t *);
int listenUDP4(int);

void doSomething();
#endif
