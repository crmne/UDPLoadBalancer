#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>
#include <sys/time.h>

/* I do not want to include stdio.h just for NULL */
#ifndef NULL
#define NULL (void *) 0
#endif
#define MAXCONNECTIONS 1
#define MAXPATHS 3
#define MAXFAIL 5
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
	struct packet_t *next;
	uint32_t numfail;
	uint32_t failid[MAXFAIL];
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
uint32_t sendPktsToApp(int, packet_t *, packet_t *, uint32_t);


void doSomething();
#endif
