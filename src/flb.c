#include <string.h>             /* memset */
#include <err.h>
#include <stdlib.h>             /* malloc */
#include "macro.h"

#define NFDS 3
#define NPKTS 2
#define HOST "127.0.0.1"
#define APP_PORT 11001
#define PEER_PORT 10001
#define PATH_PORT 9001
#define FIRST_PACKET_ID 0

#define ERR_SELECT 1, "select()"
#define ERR_MONPACK 2, "Monitor packet not understood"
#define ERR_NOFDSET 3, "Incoming data, but in no controlled fd. Strange"
typedef enum types {
    app, peer, path
} types;
int main(int argc, char *argv[])
{
    int i, socks, maxfd, fd[NFDS];
    uint32_t expected_pkt;
    packet_t *pkt[NPKTS], *recvq;
    fd_set infds, allsetinfds;

    expected_pkt = FIRST_PACKET_ID;

    recvq = NULL;

    for (i = 0; i < NPKTS; i++)
        pkt[i] = NULL;

    FD_ZERO(&allsetinfds);

    fd[app] = accept_app(listen_app(HOST, APP_PORT));
    FD_SET(fd[app], &allsetinfds);

    fd[peer] = listen_udp(HOST, PEER_PORT);
    FD_SET(fd[peer], &allsetinfds);

    fd[path] = connect_udp(HOST, PATH_PORT);

    maxfd = fd[peer];

    for (;;) {
        infds = allsetinfds;
        socks = select(maxfd + 1, &infds, NULL, NULL, NULL);
        if (socks <= 0)
            err(ERR_SELECT);
        while (socks > 0) {
            if (FD_ISSET(fd[peer], &infds)) {
                uint32_t pktid;

                pkt[peer] = (packet_t *) malloc(sizeof(packet_t));
                pktid = recv_voice_pkts(fd[peer], pkt[peer]);
                if (pktid >= expected_pkt) {
                    send_voice_pkts(fd[app], pkt[peer]);
                    expected_pkt = pktid + 1;
                }
                warnx("pktid %u, expected_pkt %u", pktid, expected_pkt);
                FD_CLR(fd[peer], &infds);
            } else if (FD_ISSET(fd[app], &infds)) {
                pkt[app] = (packet_t *) malloc(sizeof(packet_t));
                recv_voice_pkts(fd[app], pkt[app]);
                send_voice_pkts(fd[path], pkt[app]);
                FD_CLR(fd[app], &infds);
            } else
                errx(ERR_NOFDSET);
            socks--;
        }
    }
    return 0;
}
