#include <err.h>
#include "packet.h"
void insertInQ(packet_t ** pktQueue, packet_t * packet)
{
	packet_t *current, *prev;
	packet->next = NULL;
	if (*pktQueue == NULL) {
		*pktQueue = packet;
	}
	else {
		current = *pktQueue;
		prev = NULL;
		while (current != NULL) {
			if (packet->id > current->id) {
				if (current->next != NULL) {
					prev = current;
					current = (packet_t *) current->next;
				}
				else {
					current->next =
						(struct packet_t *) packet;
					current = NULL;
				}
			}
			else if (packet->id < current->id) {
				if (prev == NULL) {
					*pktQueue = packet;
				}
				else {
					prev->next =
						(struct packet_t *) packet;
				}
				packet->next = (struct packet_t *) current;
				current = NULL;
			}
			else {
				errx(1,
				     "Oh my god two packets with the same id in Queue!");
			}
		}
	}
}
packet_t *getFirstInQ(packet_t ** pktQueue)
{
	packet_t *first;
	if (*pktQueue != NULL) {
		first = *pktQueue;
		if (first->next == NULL) {
			*pktQueue = NULL;
		}
		else {
			*pktQueue = (packet_t *) first->next;
			first->next = NULL;
		}
	}
	return first;
}
