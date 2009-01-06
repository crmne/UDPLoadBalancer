#include <err.h>
#include <stdlib.h>
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
	} else {
		warnx("ACK %u, LASTSENT %u", ack->n, lastSent->id);
		/*insertInQ(&sendQueue, lastSent); ?? */
	}
}

void manageMonNack(config_t * nack, packet_t * lastSent, config_t * config)
{
	if (lastSent->id == nack->n) {
		sendVoicePkts(selectPath(config), lastSent);
	} else {
		warnx("NACK %u, LASTSENT %u", nack->n, lastSent->id);
	}
}
