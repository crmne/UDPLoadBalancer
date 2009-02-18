#ifndef __PACKET_H__
#define __PACKET_H__

#include <stdint.h>
#include <sys/time.h>
#include PROTO_H

typedef struct {
    char type;
    uint32_t n;
    uint16_t port[MAX_PATHS];
} config_t;

typedef struct {
    uint32_t id;
    struct timeval time;
    char data[PACKET_SIZE - sizeof(uint32_t) - sizeof(struct timeval)];
    struct packet_t *next;
    struct packet_additions_t pa;
} packet_t;

#endif
