#include <err.h>
#include <stdlib.h>
#include <string.h>
#include "packet.h"
#include "queue.h"
#include "comm.h"
#include "protocol.h"

int selectPath(config_t * config)
{
	return config->socket[0];
}

void manageMonAck(config_t * ack, packet_t * lastSent, packet_t * sendQueue)
{
	if (lastSent->id == ack->n) {
		free(lastSent);
	}
	else {
		warnx("ACK %u, LASTSENT %u", ack->n, lastSent->id);
		/*insertInQ(&sendQueue, lastSent); ?? */
	}
}

void manageMonNack(config_t * nack, packet_t * lastSent, config_t * config)
{
	if (lastSent->id == nack->n) {
		sendVoicePkts(selectPath(config), lastSent);
	}
	else {
		warnx("NACK %u, LASTSENT %u", nack->n, lastSent->id);
	}
}

void pa_cpy_to_pp(char *pp, struct packet_additions_t *pa)
{
	memcpy(pp, &pa->numfail, sizeof(pa->numfail));
	memcpy(pp, pa->failid, sizeof(pa->failid));
}

void pa_cpy_from_pp(struct packet_additions_t *pa, char *pp)
{
	memcpy(&pa->numfail, pp, sizeof(pa->numfail));
	memcpy(pa->failid, pp, sizeof(pa->failid));
}
