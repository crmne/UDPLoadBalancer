#ifndef __COMM_H__
#define __COMM_H__

#include "packet.h"

char recv_mon(int, config_t *);
uint32_t recv_voice_pkts(int, packet_t *, int, struct sockaddr_in *);
void send_voice_pkts(int, packet_t *, int, struct sockaddr_in *);
void send_app(int, packet_t *, packet_t **, uint32_t *);

#endif
