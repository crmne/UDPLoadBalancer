#include <unistd.h>
#include <err.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include "macro.h"

void print_routes(config_t * newcfg)
{
    int i;

    printf("New Config!");

    for (i = 0; i < newcfg->n; i++) {
        printf(" %u", newcfg->port[i]);
    }

    printf("\n");
    fflush(stdout);
}

uint32_t recv_voice_pkts(int socketfd, packet_t * packet, int type,
                         struct sockaddr_in *from)
{
    int n = 0;
    unsigned int size = 0, len = sizeof(struct sockaddr_in);
    char ppacket[PACKET_SIZE];

    memset(&ppacket, 0, sizeof(ppacket));
    memset(packet, 0, sizeof(packet_t));

    switch (type) {
    case SOCK_STREAM:
        n = read(socketfd, &ppacket, sizeof(ppacket));
        break;
    case SOCK_DGRAM:
        n = recvfrom(socketfd, &ppacket, sizeof(ppacket), 0,
                     (struct sockaddr *) from, &len);
        break;
    default:
        errx(ERR_INVALIDPROTO);
    }
    if (n == 0)
        errx(ERR_READZERO);
    if (n != sizeof(ppacket))
        err(ERR_RECV);

    memcpy(&packet->id, &ppacket, sizeof(packet->id));
    size += sizeof(packet->id);
    memcpy(&packet->time, (char *) &ppacket + size, sizeof(packet->time));
    size += sizeof(packet->time);
    memcpy(&packet->data, (char *) &ppacket + size, sizeof(packet->data));
    size += sizeof(packet->data);

#ifdef DEBUG
    warnx("Received voice packet %u from %d, size = %u, delay = %u usec",
          packet->id, type == SOCK_DGRAM ? htons(from->sin_port) : 0, size,
          timeval_age(&packet->time));
#endif
    return packet->id;
}

void send_voice_pkts(int socketfd, packet_t * packet, int type,
                     struct sockaddr_in *to)
{
    int n = 0;
    unsigned int size = 0;
    char ppacket[PACKET_SIZE];

    memset(&ppacket, 0, sizeof(ppacket));

    memcpy(&ppacket, &packet->id, sizeof(packet->id));
    size += sizeof(packet->id);
    memcpy((char *) &ppacket + size, &packet->time, sizeof(packet->time));
    size += sizeof(packet->time);
    memcpy((char *) &ppacket + size, &packet->data, sizeof(packet->data));
    size += sizeof(packet->data);

    switch (type) {
    case SOCK_STREAM:
        n = write(socketfd, &ppacket, size);
        break;
    case SOCK_DGRAM:
        n = sendto(socketfd, &ppacket, size, 0, (struct sockaddr *) to,
                   sizeof(struct sockaddr_in));
        break;
    default:
        errx(ERR_INVALIDPROTO);
    }
    if (n != size)
        err(ERR_SEND);
#ifdef DEBUG
    warnx("Sent voice packet %u to %d, size = %u, delay = %u usec",
          packet->id, type == SOCK_DGRAM ? htons(to->sin_port) : 0, size,
          timeval_age(&packet->time));
#endif

}

void send_app(int appSock, packet_t * peerPkt, packet_t ** pktQueue,
              uint32_t * expPktId)
{
    packet_t *first;

    if (peerPkt->id == *expPktId) {
        send_voice_pkts(appSock, peerPkt, SOCK_STREAM, NULL);
        *expPktId = peerPkt->id + 1;
        free(peerPkt);
        if (*pktQueue != NULL) {
            while ((first = q_extract_first(pktQueue)) != NULL
                   && first->id == *expPktId) {
                send_voice_pkts(appSock, first, SOCK_STREAM, NULL);
                *expPktId = first->id + 1;
                free(first);
            }
        }
    } else {
        q_insert(pktQueue, peerPkt);
        q_print(*pktQueue);
        /*is_not_exp_pkt(peerPkt, *expPktId); */
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
        errx(ERR_READZERO);
    if (n != sizeof(answer))
        err(ERR_RECV);
    newconfig->type = answer;
    n = read(socketfd, &newconfig->n, sizeof(newconfig->n));
    if (n != sizeof(newconfig->n))
        err(ERR_RECV);
    if (answer == 'C') {
        for (i = 0; i < newconfig->n; i++) {
            n = read(socketfd, &newconfig->port[i],
                     sizeof(newconfig->port[i]));
            if (n != sizeof(newconfig->port[i]))
                err(ERR_RECV);
        }
    }
    return answer;
}
