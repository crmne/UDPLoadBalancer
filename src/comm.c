#include <unistd.h>
#include <err.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include "macro.h"

int get_local_port(int socketfd)
{
    int ris;
    struct sockaddr_in Local;
    unsigned int addr_size;

    addr_size = sizeof(Local);
    memset((char *) &Local, 0, sizeof(Local));
    Local.sin_family = AF_INET;
    ris = getsockname(socketfd, (struct sockaddr *) &Local, &addr_size);
    if (ris < 0) {
	perror("getsockname() failed: ");
	return (0);
    } else {
	/*
	   fprintf(stderr,"IP %s port %d\n", inet_ntoa(Local.sin_addr), ntohs(Local.sin_port) );
	 */
	return (ntohs(Local.sin_port));
    }
}

uint32_t recvVoicePkts(int socketfd, packet_t * packet)
{
    int n;
    char ppacket[PACKET_SIZE];

    n = read(socketfd, &ppacket, sizeof(ppacket));
    if (n == 0)
	errx(2, "Nothing received, maybe the other end is down? Exiting.");
    if (n != sizeof(ppacket))
	err(1, "recvVoicePkts(socketfd=%d,...): read(packet)=%d", socketfd,
	    n);

    memcpy(&packet->id, &ppacket, sizeof(packet->id));
    memcpy(&packet->time, (char *) &ppacket + sizeof(packet->id),
	   sizeof(packet->time));
    memcpy(&packet->data,
	   (char *) &ppacket + sizeof(packet->id) + sizeof(packet->time),
	   sizeof(packet->data));
/*    pa_cpy_from_pp(&packet->pa,
		   (char *) &ppacket + sizeof(packet->id) +
		   sizeof(packet->time) + sizeof(packet->data));

  if (config != NULL) {
	  config->type = 'C';
	memcpy (&config->n, &);
  }*/

    packet->next = NULL;
#ifdef DEBUG
    printf("Received voice packet %u from %d, delay = %u usec\n",
	   packet->id, get_local_port(socketfd),
	   timeval_age(&packet->time));
    fflush(stdout);
#endif
    return packet->id;
}

void sendVoicePkts(int socketfd, packet_t * packet)
{
    int n;
    char ppacket[PACKET_SIZE];

    memcpy(&ppacket, &packet->id, sizeof(packet->id));
    memcpy((char *) &ppacket + sizeof(packet->id), &packet->time,
	   sizeof(packet->time));
    memcpy((char *) &ppacket + sizeof(packet->id) + sizeof(packet->time),
	   &packet->data, sizeof(packet->data));
/*    pa_cpy_to_pp((char *) &ppacket + sizeof(packet->id) +
		 sizeof(packet->time) + sizeof(packet->data), &packet->pa);*/
    n = write(socketfd, &ppacket, sizeof(ppacket));
    if (n != sizeof(ppacket))
	err(1, "sendVoicePkts(socketfd=%d,...): write(packet)%u", socketfd,
	    sizeof(ppacket));
#ifdef DEBUG
    printf("Sending voice packet %u, delay = %u usec\n", packet->id,
	   timeval_age(&packet->time));
    fflush(stdout);
#endif

}

void sendPktsToApp(int appSock, packet_t * peerPkt, packet_t ** pktQueue,
		   uint32_t * expPktId)
{
    packet_t *first;

    if (peerPkt->id == *expPktId) {
	sendVoicePkts(appSock, peerPkt);
	*expPktId = peerPkt->id + 1;
	free(peerPkt);
	if (*pktQueue != NULL) {
	    while ((first = getFirstInQ(pktQueue)) != NULL
		   && first->id == *expPktId) {
		sendVoicePkts(appSock, first);
		*expPktId = first->id + 1;
		free(first);
	    }
	}
    } else {
	insertInQ(pktQueue, peerPkt);
	printQueue(*pktQueue);
	is_not_exp_pkt(peerPkt, *expPktId);
    }
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
	err(1, "recvMonitorPkts(socketfd=%d,...): read(answer)", socketfd);
    newconfig->type = answer;
    n = read(socketfd, &newconfig->n, sizeof(newconfig->n));
    if (n != sizeof(newconfig->n))
	err(1, "recvMonitorPkts(socketfd=%d,...): read(n)", socketfd);
    if (answer == 'C') {
	for (i = 0; i < newconfig->n; i++) {
	    n = read(socketfd, &newconfig->port[i],
		     sizeof(newconfig->port[i]));
	    if (n != sizeof(newconfig->port[i]))
		err(1, "recvMonitorPkts(socketfd=%d,...): read(port[%d])",
		    socketfd, i);
	}
    }
    warnx("RECV");
    return answer;
}
