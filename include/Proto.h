#ifndef __PROTO_H__
#define __PROTO_H__

#include "Common.h"

int selectPath(config_t *);
void manageMonAck(config_t *, packet_t *, packet_t *);
void manageMonNack(config_t *, packet_t *, config_t *);

#endif
