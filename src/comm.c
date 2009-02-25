#include <unistd.h>
#include <err.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include "macro.h"

uint32_t recv_voice_pkts(int socketfd, packet_t * packet, int type,
                         struct sockaddr_in *from)
{
    int n = 0;
    char ppacket[PACKET_SIZE + sizeof(char)];
    unsigned int size = 0, len = sizeof(struct sockaddr_in), plen =
        sizeof(ppacket);

    memset(&ppacket, 0, plen);
    memset(packet, 0, sizeof(packet_t));

    switch (type) {
    case SOCK_STREAM:
        plen -= sizeof(char);
        n = read(socketfd, &ppacket, plen);
        break;
    case SOCK_DGRAM:
        n = recvfrom(socketfd, &ppacket, plen, 0, (struct sockaddr *) from,
                     &len);
        break;
    default:
        errx(ERR_INVALIDPROTO);
    }
    if (n == 0)
        errx(ERR_READZERO);
    if (n != plen)
        err(ERR_RECV);

    memcpy(&packet->id, &ppacket, sizeof(packet->id));
    size += sizeof(packet->id);
    memcpy(&packet->time, (char *) &ppacket + size, sizeof(packet->time));
    size += sizeof(packet->time);
    memcpy(&packet->data, (char *) &ppacket + size, sizeof(packet->data));
    size += sizeof(packet->data);

    if (size < plen) {
        memcpy(&packet->pa.ploss, (char *) &ppacket + size, sizeof(char));
        size += sizeof(char);
    }
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
    char ppacket[PACKET_SIZE + sizeof(char)];

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
        memcpy((char *) &ppacket + size, &packet->pa.ploss, sizeof(char));
        size += sizeof(char);
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
    }
}

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
