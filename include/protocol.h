#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__
#ifdef PROTO_H

#include "packet.h"
#include PROTO_H

struct sockaddr_in *select_path(config_t *, struct sockaddr_in *,
                                uint32_t);
void manage_ack(packet_t *);
void manage_nack(int, packet_t *, config_t *, uint32_t);

#else
#error "You must specify a protocol. To do this set the PROTOCOL variable accordingly in the Makefile. See include/protocols for a list of protocols available."
#endif
#endif
