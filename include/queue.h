#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "packet.h"

void q_insert(packet_t **, packet_t *);
packet_t *q_extract_first(packet_t **);
packet_t *q_remove(packet_t **, uint32_t);
uint32_t q_check(int, packet_t **, uint32_t);
void q_print(packet_t *);
#endif
