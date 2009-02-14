#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include "macro.h"
void q_insert(packet_t ** pktQueue, packet_t * packet)
{
    packet_t *current, *prev;

    packet->next = NULL;
    if (*pktQueue == NULL) {
        *pktQueue = packet;
    } else {
        current = *pktQueue;
        prev = NULL;
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
                errx(1,
                     "Oh my god two packets with the same id in Queue!");
            }
        }
    }
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

packet_t *q_remove(packet_t ** pktQueue, uint32_t id)
{
    packet_t *current = NULL, *prev;

    if (*pktQueue != NULL) {
        current = *pktQueue;
        prev = NULL;
        if (id == current->id)
            return q_extract_first(pktQueue);
        while (current != NULL && id != current->id) {
            prev = current;
            current = (packet_t *) current->next;
        }
        if (current != NULL) {
            prev->next = current->next;
        }
    }
    return current;
}

uint32_t removeLessQ(packet_t ** pktQueue, uint32_t id)
{
    int removed = 0;

    while (*pktQueue != NULL && id >= (*pktQueue)->id) {
        q_extract_first(pktQueue);
        removed++;
    }
    return removed;
}

void q_print(packet_t * queue)
{
    packet_t *current = queue;

    if (current != NULL) {
        fprintf(stderr, "Queue: ");
        while (current != NULL) {
            fprintf(stderr, "%d ", current->id);
            current = (packet_t *) current->next;
        }
        fprintf(stderr, "NULL\n");
    }
}

/*uint32_t
q_check (int socketfd, packet_t ** pktQueue, uint32_t expPktId)
{
  uint32_t lastone = 0;
  if (current != NULL)
  {
    if (MAXDELAY - timeval_load (current->time) <= CHECKTIME + 5)
    {
      lastone = current->id;
      while ((current = q_extract_first (pktQueue)) != NULL)
      {
	if (current->id >= expPktId)
	  send_voice_pkts (socketfd, current);
	free (current);
      }
    }
  }
  return lastone;
}*/
