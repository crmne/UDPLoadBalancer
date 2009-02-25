#include <string.h>             /* memset */
#include <err.h>
#include <stdlib.h>             /* malloc */
#include <unistd.h>             /* close */
#include <stdio.h>              /* printf, fflush -> DEBUG */
#include "macro.h"

#define NFDS 4
#define NPKTS 3
#define NCFGS 3
#define LASTDELAYS 10
#define APP_PORT 6001
#define PEER_PORT 7001
#define MON_PORT 8000
#define FIRST_PACKET_ID 9999
#define MAX_RECVQ_LENGTH 2

typedef enum types {
    mon, app, peer, path
} types;
typedef enum cfgn {
    old, new, tmp
} cfgn;

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
    uint32_t expected_pkt, last_sent_pkt, last_blah_pkt, last_freed_pkt,
        delivered, discarded, counter, lastdelays[LASTDELAYS],
        lastdelayindex, average_delay;
    packet_t *pkt[NPKTS], *recvq;
    config_t cfg[NCFGS];
    fd_set infds, allsetinfds;
    char ploss, peerloss;

    expected_pkt = FIRST_PACKET_ID;
    last_blah_pkt = last_freed_pkt = last_sent_pkt = FIRST_PACKET_ID - 1;
    delivered = discarded = average_delay = 0;
    ploss = peerloss = counter = lastdelayindex = 0;

    recvq = NULL;
    recvq_length = 0;

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
        if (delivered + discarded > 0)
            ploss =
                ((double) discarded / (double) (delivered + discarded)) *
                100;
        warnx("%u ploss", ploss);
        if (delivered >= LASTDELAYS) {
            average_delay = 0;
            for (i = 0; i < LASTDELAYS; i++)
                average_delay += lastdelays[i];
            average_delay /= LASTDELAYS;
        }
        if (recvq_length >= MAX_RECVQ_LENGTH) {
            while (recvq_length > 0) {
                packet_t *a_pack = q_extract_first(&recvq);
                recvq_length--;
                if (a_pack->id >= expected_pkt) {
                    send_voice_pkts(fd[app], a_pack, SOCK_STREAM, NULL);
                    lastdelays[lastdelayindex] =
                        timeval_age(&a_pack->time);
                    if (lastdelays[lastdelayindex] < MAX_DELAY) {
                        lastdelayindex = (lastdelayindex + 1) % LASTDELAYS;
                        delivered++;
                        discarded += a_pack->id - last_blah_pkt - 1;
                        last_blah_pkt = a_pack->id;
                    }
                    expected_pkt = a_pack->id + 1;
                } else
                    discarded++;
                free(a_pack);
            }
        }
        while (socks > 0) {
            socks--;
            if (FD_ISSET(fd[mon], &infds)) {
                char answ = recv_mon(fd[mon], &cfg[tmp]);
                switch (answ) {
                case 'A':
                    if (pkt[app]->id == last_sent_pkt
                        && pkt[app]->id != last_freed_pkt) {
                        manage_ack(pkt[app]);
                        last_freed_pkt = pkt[app]->id;
                    }
                    break;
                case 'N':
                    if (pkt[app]->id == last_sent_pkt) {
                        manage_nack(fd[peer], pkt[app], &cfg[new],
                                    average_delay);
                    }
                    break;
                case 'C':
                    cfg[old] = cfg[new];
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
                pkt[app]->pa.ploss = ploss;
                send_voice_pkts(fd[peer], pkt[app], SOCK_DGRAM,
                                select_path(&cfg[new], &to,
                                            average_delay));
                counter++;
                last_sent_pkt = pkt[app]->id;
                FD_CLR(fd[app], &infds);
            } else if (FD_ISSET(fd[peer], &infds)) {
                uint32_t pktid;
                struct sockaddr_in from;
                pkt[peer] = (packet_t *) malloc(sizeof(packet_t));
                if (pkt[app] == NULL)
                    err(ERR_MALLOC);
                pktid =
                    recv_voice_pkts(fd[peer], pkt[peer], SOCK_DGRAM,
                                    &from);
                peerloss = pkt[peer]->pa.ploss;
                if (pktid == expected_pkt) {
                    send_voice_pkts(fd[app], pkt[peer], SOCK_STREAM, NULL);
                    lastdelays[lastdelayindex] =
                        timeval_age(&pkt[peer]->time);
                    if (lastdelays[lastdelayindex] < MAX_DELAY) {
                        lastdelayindex = (lastdelayindex + 1) % LASTDELAYS;
                        delivered++;
                        discarded += pktid - last_blah_pkt - 1;
                        last_blah_pkt = pktid;
                    }
                    free(pkt[peer]);

                    expected_pkt++;
                } else if (pktid > expected_pkt) {
                    recvq_length += q_insert(&recvq, pkt[peer]);
                }
#ifdef DEBUG
                warnx("pktid %u, expected_pkt %u", pktid, expected_pkt);
#endif
                FD_CLR(fd[peer], &infds);
            } else
                errx(ERR_NOFDSET);
        }
    }
    return 127;
}
