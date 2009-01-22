#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "packet.h"

void insertInQ(packet_t **, packet_t *);
packet_t *getFirstInQ(packet_t **);
packet_t *removeFromQ(packet_t **, uint32_t);
uint32_t checkPktQueue(int, packet_t **, uint32_t);
#endif
