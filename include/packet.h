#ifndef __PACKET_H__
#define __PACKET_H__

#include <stdint.h>
#include "utils.h"
#include PROTO_H

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
	struct packet_additions_t pa;
} packet_t;

#endif
