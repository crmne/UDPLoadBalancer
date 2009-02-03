#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__
#ifdef PROTO_H

#include "packet.h"
#include PROTO_H

int select_path(config_t *);
void manage_ack(config_t *, packet_t *);
void manage_nack(config_t *, packet_t *, config_t *);
void pa_cpy_to_pp(char *, struct packet_additions_t *);
void pa_cpy_from_pp(struct packet_additions_t *, char *);
void is_not_exp_pkt(packet_t *, uint32_t);

#else
#error "You must specify a protocol. To do this set the PROTOCOL variable accordingly in the Makefile. See include/protocols for a list of protocols available."
#endif
#endif
