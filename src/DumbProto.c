#include <err.h>
#include <stdlib.h>
#include "Common.h"
#include "Queues.h"
#include "Proto.h"

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
		errx("ACK %u, LASTSENT %u", ack->n, lastSent->id);
		/*insertInQ(&sendQueue, lastSent); */
	}
}

void manageMonNack(config_t * nack, packet_t * lastSent, config_t * config)
{
	if (lastSent->id != nack->n)
		errx(255, "CAZZO!");
	else {
		sendVoicePkts(selectPath(config), lastSent);
		/*free(lastSent); */
	}
}
