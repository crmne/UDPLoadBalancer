#include <string.h>             /* memset */
#include <err.h>
#include <stdlib.h>             /* malloc */
#include "macro.h"

#define NFDS 3
#define NPKTS 2
#define NCFGS 3
#define HOST "127.0.0.1"
#define APP_PORT 11001
#define PEER_PORT 10001
#define PATH_PORT 9001
#define FIRST_PACKET_ID 0
#define MAX_RECVQ_LENGTH 2

#define ERR_SELECT 1, "select()"
#define ERR_MONPACK 2, "Monitor packet not understood"
#define ERR_NOFDSET 3, "Incoming data, but in no controlled fd. Strange"
typedef enum types {
    app, peer, path
} types;
typedef enum cfgn {
    old, new, tmp
} cfgn;
int main(int argc, char *argv[])
{
    int i, socks, maxfd, fd[NFDS], recvq_length;
    uint32_t expected_pkt;
    packet_t *pkt[NPKTS], *recvq;
    config_t cfg[NCFGS];
    fd_set infds, allsetinfds;

    expected_pkt = FIRST_PACKET_ID;

    recvq = NULL;
    recvq_length = 0;

    for (i = 0; i < NPKTS; i++)
        pkt[i] = NULL;

    for (i = 0; i < NCFGS; i++) {
        cfg[i].type = 0;
        cfg[i].n = 0;
    }


    FD_ZERO(&allsetinfds);

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
            if (FD_ISSET(fd[peer], &infds)) {
                uint32_t pktid;

                pkt[peer] = (packet_t *) calloc(1, sizeof(packet_t));
                pktid = recv_voice_pkts(fd[peer], pkt[peer]);
                if (pkt[peer]->pa.n > 0) {
                    cfg[tmp].n = pkt[peer]->pa.n;
                    memcpy(&cfg[tmp].port, &pkt[peer]->pa.port,
                           sizeof(cfg[tmp].port));
                    cfg[old] = cfg[new];
                    cfg[new] = cfg[tmp];
                    reconf_routes(&cfg[old], &cfg[new], 1000);
                    pkt[peer]->pa.n = 0;
                }
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
            } else if (FD_ISSET(fd[app], &infds)) {
                pkt[app] = (packet_t *) calloc(1, sizeof(packet_t));
                /*pkt[app]->pa.n = 0; */
                recv_voice_pkts(fd[app], pkt[app]);
                fd[path] = select_path(&cfg[new]);

                send_voice_pkts(fd[path], pkt[app]);
                FD_CLR(fd[app], &infds);
            } else
                errx(ERR_NOFDSET);
        }
    }
    return 0;
}
