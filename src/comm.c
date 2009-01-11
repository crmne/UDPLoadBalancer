#include <unistd.h>
#include <err.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "packet.h"
#include "protocol.h"
#include "queue.h"
#include "utils.h"

uint32_t recvVoicePkts(int socketfd, packet_t * packet)
{
	int n;
	char ppacket[PKTSIZE];
	n = read(socketfd, &ppacket, sizeof(ppacket));
	if (n == 0)
		errx(2,
		     "Nothing received, maybe the other end is down? Exiting.");
	if (n != sizeof(ppacket))
		err(1,
		    "recvVoicePkts(socketfd=%d,...): read(packet)=%d",
		    socketfd, n);
	memcpy(&packet->id, &ppacket, sizeof(packet->id));
	memcpy(&packet->time, (char *) &ppacket + sizeof(packet->id),
	       sizeof(packet->time));
	memcpy(&packet->data,
	       (char *) &ppacket + sizeof(packet->id) + sizeof(packet->time),
	       sizeof(packet->data));
	pa_cpy_from_pp(&packet->pa,
		       (char *) &ppacket + sizeof(packet->id) +
		       sizeof(packet->time) + sizeof(packet->data));
	packet->next = NULL;
#ifdef DEBUG
	printf("Received voice packet %u, delay = %f msec\n", packet->id, calcDelay(packet->time));	/* TODO: from who? */
	fflush(stdout);
#endif
	return packet->id;
}

void sendVoicePkts(int socketfd, packet_t * packet)
{
	int n;
	char ppacket[PKTSIZE];
	memcpy(&ppacket, &packet->id, sizeof(packet->id));
	memcpy((char *) &ppacket + sizeof(packet->id), &packet->time,
	       sizeof(packet->time));
	memcpy((char *) &ppacket + sizeof(packet->id) + sizeof(packet->time),
	       &packet->data, sizeof(packet->data));
	pa_cpy_to_pp((char *) &ppacket + sizeof(packet->id) +
		     sizeof(packet->time) + sizeof(packet->data),
		     &packet->pa);
	n = write(socketfd, &ppacket, sizeof(ppacket));
	if (n != sizeof(ppacket))
		err(1, "sendVoicePkts(socketfd=%d,...): write(packet)%lu",
		    socketfd, sizeof(ppacket));
#ifdef DEBUG
	printf("Sending voice packet %u, delay = %f msec\n", packet->id,
	       calcDelay(packet->time));
	fflush(stdout);
#endif

}

uint32_t sendPktsToApp(int appSock, packet_t * peerPkt, packet_t * pktQueue,
		       uint32_t expPktId)
{
	packet_t *first;
	uint32_t i = 0;
	if (peerPkt->id == expPktId) {
		sendVoicePkts(appSock, peerPkt);
		free(peerPkt);
		i = 1;
		if (pktQueue != NULL) {
			while ((first = getFirstInQ(&pktQueue)) != NULL
			       && first->id == expPktId + i) {
				sendVoicePkts(appSock, first);
				free(first);
				i++;
			}
		}
	}
	else {
		insertInQ(&pktQueue, peerPkt);
	}
	return i;
}

/** Receive packets from monitor
 * @param socketfd
 * @param buffer
 * @return C for config, A for ack, N for nack
 */
char recvMonitorPkts(int socketfd, config_t * newconfig)
{
	int n, i;
	char answer;
	n = read(socketfd, &answer, sizeof(answer));
	if (n == 0)
		errx(2, "Nothing received, maybe Monitor is down? Exiting.");
	if (n != sizeof(answer))
		err(1, "recvMonitorPkts(socketfd=%d,...): read(answer)",
		    socketfd);
	newconfig->type = answer;
	n = read(socketfd, &newconfig->n, sizeof(newconfig->n));
	if (n != sizeof(newconfig->n))
		err(1, "recvMonitorPkts(socketfd=%d,...): read(n)", socketfd);
#ifdef DEBUG
	printf("Received new ");
#endif
	switch (answer) {
	case 'C':
#ifdef DEBUG
		printf("configuration: ");
#endif
		for (i = 0; i < newconfig->n; i++) {
			n = read(socketfd, &newconfig->port[i],
				 sizeof(newconfig->port[i]));
			if (n != sizeof(newconfig->port[i]))
				err(1,
				    "recvMonitorPkts(socketfd=%d,...): read(port[%d])",
				    socketfd, i);
#ifdef DEBUG
			printf("%u ", newconfig->port[i]);
#endif
		}

		break;
	case 'A':
#ifdef DEBUG
		printf("ack ");
#endif
		break;
	case 'N':
		/* read read */
#ifdef DEBUG
		printf("nack ");
#endif
		break;
	default:
		errx(1,
		     "recvMonitorPkts(socketfd=%d,...): read(): Invalid packet",
		     socketfd);
	}
#ifdef DEBUG
	printf("from Monitor\n");
	fflush(stdout);
#endif
	return answer;
}
