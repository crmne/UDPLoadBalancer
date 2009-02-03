#include <string.h>		/* memset */
#include <err.h>
#include <stdlib.h>		/* malloc */
#include <unistd.h>		/* close */
#include <stdio.h>		/* printf, fflush -> DEBUG */
#include "macro.h"

#define NFDS 4
#define NPKTS 3
#define NCFGS 3
#define HOST "127.0.0.1"
#define APP_PORT 6001
#define PEER_PORT 7001
#define MON_PORT 8000
#define FIRST_PACKET_ID 9999

#define ERR_SELECT 1, "select()"
#define ERR_MONPACK 2, "Monitor packet not understood"
#define ERR_NOFDSET 3, "Incoming data, but in no controlled fd. Strange"
typedef enum types { mon, app, peer, path } types;
typedef enum cfgn { old, new, tmp } cfgn;
int main(int argc, char *argv[])
{
    int i, socks, maxfd, fd[NFDS];
    uint32_t expected_pkt;
    packet_t *pkt[NPKTS], *recvq;
    config_t cfg[NCFGS];
    fd_set infds, allsetinfds;

    expected_pkt = FIRST_PACKET_ID;

    recvq = NULL;

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

    for (;;)
    {
	infds = allsetinfds;
	socks = select(maxfd + 1, &infds, NULL, NULL, NULL);
	if (socks <= 0)
	    err(ERR_SELECT);
	while (socks > 0)
	{
	    if (FD_ISSET(fd[mon], &infds))
	    {
		char answ = recv_mon(fd[mon], &cfg[tmp]);

		switch (answ)
		{
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
		    break;
		default:
		    errx(ERR_MONPACK);
		}

		FD_CLR(fd[mon], &infds);
	    } else if (FD_ISSET(fd[app], &infds))
	    {
		pkt[app] = (packet_t *) malloc(sizeof(packet_t));
		recv_voice_pkts(fd[app], pkt[app]);
		fd[path] = select_path(&cfg[new]);
		send_voice_pkts(fd[path], pkt[app]);
		FD_CLR(fd[app], &infds);
	    } else if (FD_ISSET(fd[peer], &infds))
	    {
		uint32_t pktid;

		pkt[peer] = (packet_t *) malloc(sizeof(packet_t));
		pktid = recv_voice_pkts(fd[peer], pkt[peer]);
		if (pktid >= expected_pkt)
		{
		    send_voice_pkts(fd[app], pkt[peer]);
		    expected_pkt = pktid + 1;
		}
		warnx("pktid %u, expected_pkt %u", pktid, expected_pkt);
		FD_CLR(fd[peer], &infds);
	    } else
		errx(ERR_NOFDSET);
	    socks--;
	}
    }
    return 0;
}
