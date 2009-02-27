#include <string.h>
#include <err.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "macro.h"

#define NFDS 3
#define NPKTS 3
#define NCFGS 2
#define NPKTIDS 4
#define NCOUNTERS 2
#define LASTDELAYS 10
#define APP_PORT 6001
#define PEER_PORT 7001
#define MON_PORT 8000
#define FIRST_PACKET_ID 9999
#define MAX_RECVQ_LENGTH 2
#define MAX_ACCEPTABLE_DELAY MAX_DELAY * 8 / 15

typedef enum types {
    mon, app, peer
} types;
typedef enum cfgn {
    new, tmp
} cfgn;
typedef enum pktids {
    expected, sentpeer, sentapp, freed
} pktids;
typedef enum counters {
    delivered, discarded
} counters;

unsigned int path = 0;

struct sockaddr_in *select_path(config_t * config, struct sockaddr_in *to,
                                uint32_t average_delay)
{
    to->sin_family = AF_INET;
    to->sin_addr.s_addr = inet_addr(HOST);
    if (to->sin_addr.s_addr < 0)
        err(ERR_INETADDR);
    if (config->n != 0) {
        if (average_delay > (MAX_ACCEPTABLE_DELAY))
            path = (path + 1) % config->n;
        to->sin_port = htons(config->port[path]);

    } else {
        warnx("No available routes!");
        to->sin_port = 0;
    }
    return to;
}

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

int main(int argc, char *argv[])
{
    int i, socks, maxfd, fd[NFDS], recvq_length;
    uint32_t pktid[NPKTIDS], count[NCOUNTERS];
    char losscount;
    uint32_t lastdelays[LASTDELAYS], lastdelayindex, averagedelay;
    packet_t *pkt[NPKTS], *recvq;
    config_t cfg[NCFGS];
    fd_set infds, allsetinfds;

    averagedelay = 0;
    losscount = 0;
    lastdelayindex = 0;
    recvq = NULL;
    recvq_length = 0;

    pktid[expected] = FIRST_PACKET_ID;

    for (i = 1; i < NPKTIDS; i++)
        pktid[i] = FIRST_PACKET_ID - 1;

    for (i = 0; i < NCOUNTERS; i++)
        count[i] = 0;

    for (i = 0; i < NPKTS; i++)
        pkt[i] = NULL;

    for (i = 0; i < NCFGS; i++) {
        cfg[i].type = 0;
        cfg[i].n = 0;
    }

    FD_ZERO(&allsetinfds);

    fd[mon] = connect_mon(HOST, MON_PORT);
    FD_SET(fd[mon], &allsetinfds);

    fd[app] = accept_app(listen_app(HOST, APP_PORT));
    FD_SET(fd[app], &allsetinfds);

    fd[peer] = bind_udp(HOST, PEER_PORT);
    FD_SET(fd[peer], &allsetinfds);

    maxfd = fd[peer];

    for (;;) {
        infds = allsetinfds;
        socks = select(maxfd + 1, &infds, NULL, NULL, NULL);
        if (socks <= 0)
            err(ERR_SELECT);
        if (count[delivered] + count[discarded] > 0)
            losscount =
                ((double) count[discarded] /
                 (double) (count[delivered] + count[discarded])) * 100;
        if (count[delivered] >= LASTDELAYS) {
            averagedelay = 0;
            for (i = 0; i < LASTDELAYS; i++)
                averagedelay += lastdelays[i];
            averagedelay /= LASTDELAYS;
        }
        if (recvq_length >= MAX_RECVQ_LENGTH) {
            while (recvq_length > 0) {
                packet_t *a_pack = q_extract_first(&recvq);
                recvq_length--;
                if (a_pack->id >= pktid[expected]) {
                    send_voice_pkts(fd[app], a_pack, SOCK_STREAM, NULL);
                    lastdelays[lastdelayindex] =
                        timeval_age(&a_pack->time);
                    if (lastdelays[lastdelayindex] < MAX_DELAY) {
                        lastdelayindex = (lastdelayindex + 1) % LASTDELAYS;
                        count[delivered]++;
                        count[discarded] +=
                            a_pack->id - pktid[sentapp] - 1;
                        pktid[sentapp] = a_pack->id;
                    }
                    pktid[expected] = a_pack->id + 1;
                } else
                    count[discarded]++;
                free(a_pack);
            }
        }
        while (socks > 0) {
            socks--;
            if (FD_ISSET(fd[mon], &infds)) {
                char answ = recv_mon(fd[mon], &cfg[tmp]);
                switch (answ) {
                case 'A':
                    if (pkt[app]->id == pktid[sentpeer]
                        && pkt[app]->id != pktid[freed]) {
                        free(pkt[app]);
                        pktid[freed] = pkt[app]->id;
                    }
                    break;
                case 'N':
                    if (pkt[app]->id == pktid[sentpeer]) {
                        struct sockaddr_in to;
                        send_voice_pkts(fd[peer], pkt[app], SOCK_DGRAM,
                                        select_path(&cfg[new], &to,
                                                    averagedelay));
                    }
                    break;
                case 'C':
                    cfg[new] = cfg[tmp];
                    print_routes(&cfg[new]);
                    break;
                default:
                    errx(ERR_MONPACK);
                }
                FD_CLR(fd[mon], &infds);
            } else if (FD_ISSET(fd[app], &infds)) {
                struct sockaddr_in to;
                pkt[app] = (packet_t *) malloc(sizeof(packet_t));
                if (pkt[app] == NULL)
                    err(ERR_MALLOC);
                recv_voice_pkts(fd[app], pkt[app], SOCK_STREAM, NULL);
                pkt[app]->ploss = losscount;
                send_voice_pkts(fd[peer], pkt[app], SOCK_DGRAM,
                                select_path(&cfg[new], &to, averagedelay));
                pktid[sentpeer] = pkt[app]->id;
                FD_CLR(fd[app], &infds);
            } else if (FD_ISSET(fd[peer], &infds)) {
                uint32_t recvpktid;
                struct sockaddr_in from;
                pkt[peer] = (packet_t *) malloc(sizeof(packet_t));
                if (pkt[app] == NULL)
                    err(ERR_MALLOC);
                recvpktid =
                    recv_voice_pkts(fd[peer], pkt[peer], SOCK_DGRAM,
                                    &from);
                losscount = pkt[peer]->ploss;
                if (recvpktid == pktid[expected]) {
                    send_voice_pkts(fd[app], pkt[peer], SOCK_STREAM, NULL);
                    lastdelays[lastdelayindex] =
                        timeval_age(&pkt[peer]->time);
                    if (lastdelays[lastdelayindex] < MAX_DELAY) {
                        lastdelayindex = (lastdelayindex + 1) % LASTDELAYS;
                        count[delivered]++;
                        count[discarded] += recvpktid - pktid[sentapp] - 1;
                        pktid[sentapp] = recvpktid;
                    }
                    free(pkt[peer]);

                    pktid[expected]++;
                } else if (recvpktid > pktid[expected]) {
                    recvq_length += q_insert(&recvq, pkt[peer]);
                }
#ifdef DEBUG
                warnx("received pkt %u, expected %u", recvpktid,
                      pktid[expected]);
#endif
                FD_CLR(fd[peer], &infds);
            } else
                errx(ERR_NOFDSET);
        }
    }
    return 127;
}
