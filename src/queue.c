#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include "macro.h"
void insertInQ(packet_t ** pktQueue, packet_t * packet)
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
packet_t *getFirstInQ(packet_t ** pktQueue)
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

packet_t *removeFromQ(packet_t ** pktQueue, uint32_t id)
{
    packet_t *current = NULL, *prev;

    if (*pktQueue != NULL) {
	current = *pktQueue;
	prev = NULL;
	if (id == current->id)
	    return getFirstInQ(pktQueue);
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
	getFirstInQ(pktQueue);
	removed++;
    }
    return removed;
}

void printQueue(packet_t * queue)
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
checkPktQueue (int socketfd, packet_t ** pktQueue, uint32_t expPktId)
{
  uint32_t lastone = 0;
  if (current != NULL)
  {
    if (MAXDELAY - timeval_load (current->time) <= CHECKTIME + 5)
    {
      lastone = current->id;
      while ((current = getFirstInQ (pktQueue)) != NULL)
      {
	if (current->id >= expPktId)
	  sendVoicePkts (socketfd, current);
	free (current);
      }
    }
  }
  return lastone;
}*/
