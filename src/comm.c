#include <unistd.h>
#include <err.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include "macro.h"

void reconf_routes(config_t * oldcfg, config_t * newcfg)
{
    int i;

    printf("New Config!");
    for (i = 0; i < oldcfg->n; i++)
    {
	close(oldcfg->socket[i]);
    }
    for (i = 0; i < newcfg->n; i++)
    {
	newcfg->socket[i] = connect_udp("127.0.0.1", newcfg->port[i]);
	printf(" %u", newcfg->port[i]);
    }
    printf("\n");
    fflush(stdout);
}

int get_local_port(int socketfd)
{
    int ris;
    struct sockaddr_in local;
    unsigned int addr_size;

    addr_size = sizeof(local);
    memset((char *) &local, 0, sizeof(local));
    local.sin_family = AF_INET;
    ris = getsockname(socketfd, (struct sockaddr *) &local, &addr_size);
    if (ris < 0)
    {
	warn("getsockname()");
	return (0);
    }
    return (ntohs(local.sin_port));
}

uint32_t recv_voice_pkts(int socketfd, packet_t * packet)
{
    int n;
    char ppacket[PACKET_SIZE];

    n = read(socketfd, &ppacket, sizeof(ppacket));
    if (n == 0)
	errx(2, "Nothing received, maybe the other end is down? Exiting.");
    if (n != sizeof(ppacket))
	err(1, "recv_voice_pkts(socketfd=%d,...): read(packet)=%d",
	    socketfd, n);

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

void send_voice_pkts(int socketfd, packet_t * packet)
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
	err(1, "send_voice_pkts(socketfd=%d,...): write(packet)%u",
	    socketfd, sizeof(ppacket));
#ifdef DEBUG
    printf("Sending voice packet %u, delay = %u usec\n", packet->id,
	   timeval_age(&packet->time));
    fflush(stdout);
#endif

}

void send_app(int appSock, packet_t * peerPkt, packet_t ** pktQueue,
	      uint32_t * expPktId)
{
    packet_t *first;

    if (peerPkt->id == *expPktId)
    {
	send_voice_pkts(appSock, peerPkt);
	*expPktId = peerPkt->id + 1;
	free(peerPkt);
	if (*pktQueue != NULL)
	{
	    while ((first = q_extract_first(pktQueue)) != NULL
		   && first->id == *expPktId)
	    {
		send_voice_pkts(appSock, first);
		*expPktId = first->id + 1;
		free(first);
	    }
	}
    } else
    {
	q_insert(pktQueue, peerPkt);
	q_print(*pktQueue);
	is_not_exp_pkt(peerPkt, *expPktId);
    }
}

/** Receive packets from monitor
 * @param socketfd
 * @param buffer
 * @return C for config, A for ack, N for nack
 */
char recv_mon(int socketfd, config_t * newconfig)
{
    int n, i;
    char answer;

    n = read(socketfd, &answer, sizeof(answer));
    if (n == 0)
	errx(2, "Nothing received, maybe Monitor is down? Exiting.");
    if (n != sizeof(answer))
	err(1, "recv_mon(socketfd=%d,...): read(answer)", socketfd);
    newconfig->type = answer;
    n = read(socketfd, &newconfig->n, sizeof(newconfig->n));
    if (n != sizeof(newconfig->n))
	err(1, "recv_mon(socketfd=%d,...): read(n)", socketfd);
    if (answer == 'C')
    {
	for (i = 0; i < newconfig->n; i++)
	{
	    n = read(socketfd, &newconfig->port[i],
		     sizeof(newconfig->port[i]));
	    if (n != sizeof(newconfig->port[i]))
		err(1, "recv_mon(socketfd=%d,...): read(port[%d])",
		    socketfd, i);
	}
    }
    warnx("RECV");
    return answer;
}
