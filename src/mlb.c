#include <string.h>             /* memset */
#include <err.h>
#include <stdlib.h>             /* malloc */
#include <unistd.h>             /* close */
#include <stdio.h>              /* printf, fflush -> DEBUG */
#include "macro.h"

#define NFDS 4
#define NPKTS 3
#define NCFGS 3
#define HOST "127.0.0.1"
#define APP_PORT 6001
#define PEER_PORT 7001
#define MON_PORT 8000
#define FIRST_PACKET_ID 9999
#define MAX_RECVQ_LENGTH 2
#define CFG_SEND_RETRY 2

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
    int i, socks, maxfd, fd[NFDS], recvq_length, cfg_send_retry;
    uint32_t expected_pkt;
    packet_t *pkt[NPKTS], *recvq;
    config_t cfg[NCFGS];
    fd_set infds, allsetinfds;

    expected_pkt = FIRST_PACKET_ID;

    recvq = NULL;
    recvq_length = 0;
    cfg_send_retry = 0;

    for (i = 0; i < NPKTS; i++)
        pkt[i] = NULL;

    for (i = 0; i < NCFGS; i++)
        memset(&cfg[i], 0, sizeof(cfg[i]));

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
                recvq_length--;
                send_voice_pkts(fd[app], a_pack);
                expected_pkt = a_pack->id + 1;
                free(a_pack);
            }
        }
        while (socks > 0) {
            socks--;
            if (FD_ISSET(fd[mon], &infds)) {
                char answ = recv_mon(fd[mon], &cfg[tmp]);

                switch (answ) {
                case 'A':
                    manage_ack(&cfg[tmp], pkt[app]);
                    warnx("ACK");
                    break;
                case 'N':
                    manage_nack(&cfg[tmp], pkt[app], &cfg[new]);
                    warnx("NACK");
                    break;
                case 'C':
                    cfg[old] = cfg[new];
                    cfg[new] = cfg[tmp];
                    reconf_routes(&cfg[old], &cfg[new]);
                    cfg_send_retry = CFG_SEND_RETRY;
                    break;
                default:
                    errx(ERR_MONPACK);
                }

                FD_CLR(fd[mon], &infds);
            } else if (FD_ISSET(fd[app], &infds)) {
                pkt[app] = (packet_t *) calloc(1, sizeof(packet_t));
                /*pkt[app]->pa.n = 0; */
                recv_voice_pkts(fd[app], pkt[app]);

                fd[path] = select_path(&cfg[new]);
                /* select path */
                /*fd[path] =
                   cfg[new].
                   socket[(((pkt[app]->id / (25 / cfg[new].n)) - 1 ) %
                   cfg[new].n)]; */
                if (cfg_send_retry > 0) {
                    cfg_send_retry--;
                    pkt[app]->pa.n = cfg[new].n;
                    memcpy(pkt[app]->pa.port, cfg[new].port,
                           sizeof(cfg[new].port));
                }
                send_voice_pkts(fd[path], pkt[app]);
                FD_CLR(fd[app], &infds);
            } else if (FD_ISSET(fd[peer], &infds)) {
                uint32_t pktid;

                pkt[peer] = (packet_t *) calloc(1, sizeof(packet_t));
                /*pkt[peer]->pa.n = 0; */
                pktid = recv_voice_pkts(fd[peer], pkt[peer]);
                if (pktid == expected_pkt) {
                    send_voice_pkts(fd[app], pkt[peer]);
                    free(pkt[peer]);
                    expected_pkt++;
                } else if (pktid > expected_pkt) {
                    q_insert(&recvq, pkt[peer]);
                    recvq_length++;
                }
                warnx("pktid %u, expected_pkt %u", pktid, expected_pkt);
                FD_CLR(fd[peer], &infds);
            } else
                errx(ERR_NOFDSET);
        }
    }
    return 0;
}
