#include <string.h>             /* memset */
#include <err.h>
#include <stdlib.h>             /* malloc */
#include <unistd.h>             /* close */
#include <stdio.h>              /* printf, fflush -> DEBUG */
#include "macro.h"

#define NFDS 4
#define NPKTS 3
#define NCFGS 3
#define APP_PORT 6001
#define PEER_PORT 7001
#define MON_PORT 8000
#define FIRST_PACKET_ID 9999
#define MAX_RECVQ_LENGTH 2

#define ERR_SELECT 1, "select()"
#define ERR_MONPACK 2, "Monitor packet not understood"
#define ERR_NOFDSET 3, "Incoming data, but in no controlled fd. Strange"
typedef enum types {
    mon, app, peer, path
} types;
typedef enum cfgn {
    old, new, tmp
} cfgn;
int main(int argc, char *argv[])
{
    int i, socks, maxfd, fd[NFDS], recvq_length;
    uint32_t expected_pkt, last_sent_pkt, last_freed_pkt;
    packet_t *pkt[NPKTS], *recvq;
    config_t cfg[NCFGS];
    fd_set infds, allsetinfds;

    expected_pkt = FIRST_PACKET_ID;
    last_sent_pkt = FIRST_PACKET_ID - 1;

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

    fd[peer] = listen_udp(HOST, PEER_PORT);
    FD_SET(fd[peer], &allsetinfds);

    maxfd = fd[peer];

    for (;;) {
        infds = allsetinfds;
        socks = select(maxfd + 1, &infds, NULL, NULL, NULL);
        if (socks <= 0)
            err(ERR_SELECT);
        if (recvq_length >= MAX_RECVQ_LENGTH) {
            while (recvq_length > 0) {
                packet_t *a_pack = q_extract_first(&recvq);
                warnx("EXTRACTED PKT %u", a_pack->id);
                recvq_length--;
                if (a_pack->id >= expected_pkt) {
                    send_voice_pkts(fd[app], a_pack, SOCK_STREAM, NULL);
                    expected_pkt = a_pack->id + 1;
                }
                warnx("free(%u)", a_pack->id);
                free(a_pack);
            }
        }
        while (socks > 0) {
            socks--;
            if (FD_ISSET(fd[mon], &infds)) {
                char answ = recv_mon(fd[mon], &cfg[tmp]);

                switch (answ) {
                case 'A':
                    warnx("ACK");
                    /* defensive programming! */
                    if (pkt[app]->id == last_sent_pkt && pkt[app]->id != last_freed_pkt) {
                        warnx("free(%u)", pkt[app]->id);
                        manage_ack(pkt[app]);
			last_freed_pkt = pkt[app]->id;
                    } else
                        warnx("DOUBLE ACK!");
                    break;
                case 'N':
                    warnx("NACK");
                    if (pkt[app]->id == last_sent_pkt) {
                        manage_nack(fd[peer], pkt[app], &cfg[new]);
                    } else
                        warnx("DOUBLE NACK!");
                    break;
                case 'C':
                    warnx("CONFIG!");
                    cfg[old] = cfg[new];
                    cfg[new] = cfg[tmp];
                    reconf_routes(&cfg[old], &cfg[new]);
                    break;
                default:
                    errx(ERR_MONPACK);
                }
                FD_CLR(fd[mon], &infds);
            } else if (FD_ISSET(fd[app], &infds)) {
                struct sockaddr_in to;
                pkt[app] = (packet_t *) malloc(sizeof(packet_t));
                recv_voice_pkts(fd[app], pkt[app], SOCK_STREAM, NULL);

                send_voice_pkts(fd[peer], pkt[app], SOCK_DGRAM,
                                select_path(&cfg[new], &to));
                last_sent_pkt = pkt[app]->id;
                FD_CLR(fd[app], &infds);
            } else if (FD_ISSET(fd[peer], &infds)) {
                uint32_t pktid;
                struct sockaddr_in from;
                pkt[peer] = (packet_t *) malloc(sizeof(packet_t));
                pktid =
                    recv_voice_pkts(fd[peer], pkt[peer], SOCK_DGRAM,
                                    &from);

                if (pktid == expected_pkt) {
                    send_voice_pkts(fd[app], pkt[peer], SOCK_STREAM, NULL);
                    warnx("free(%u)", pkt[peer]->id);
                    free(pkt[peer]);
                    expected_pkt++;
                } else if (pktid > expected_pkt) {
                    recvq_length += q_insert(&recvq, pkt[peer]);
                }
                warnx("pktid %u, expected_pkt %u", pktid, expected_pkt);
                FD_CLR(fd[peer], &infds);
            } else
                errx(ERR_NOFDSET);
        }
    }
    return 0;
}
