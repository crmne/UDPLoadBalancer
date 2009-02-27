#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include "macro.h"
char q_insert(packet_t ** pktQueue, packet_t * packet)
{
    char inserted = 1;
    packet_t *current, *prev;

    current = *pktQueue;
    prev = NULL;

    if (current == NULL) {
        packet->next = NULL;
        *pktQueue = packet;
    } else {
        while (current != NULL) {
            if (packet->id > current->id) {
                if (current->next != NULL) {
                    prev = current;
                    current = (packet_t *) current->next;
                } else {
                    current->next = (struct packet_t *) packet;
                    current = NULL;
                }
            } else if (packet->id < current->id) {
                if (prev == NULL) {
                    *pktQueue = packet;
                } else {
                    prev->next = (struct packet_t *) packet;
                }
                packet->next = (struct packet_t *) current;
                current = NULL;
            } else {
#ifdef DEBUG
                warnx(WARN_2PKTQ);
#endif
                current = NULL;
                inserted = 0;
            }
        }
    }
    return inserted;
}

packet_t *q_extract_first(packet_t ** pktQueue)
{
    packet_t *first = NULL;

    if (*pktQueue != NULL) {
        first = *pktQueue;
        if (first->next == NULL) {
            *pktQueue = NULL;
        } else {
            *pktQueue = (packet_t *) first->next;
            first->next = NULL;
        }
    }
    return first;
}

void q_print(packet_t * queue)
{
    packet_t *current = queue;

    fprintf(stderr, "Queue: ");
    while (current != NULL) {
        fprintf(stderr, "%d ", current->id);
        current = (packet_t *) current->next;
    }
    fprintf(stderr, "NULL\n");
}
