#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include "packet.h"

int selectPath(config_t *);
void manageMonAck(config_t *, packet_t *, packet_t *);
void manageMonNack(config_t *, packet_t *, config_t *);

#endif
