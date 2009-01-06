#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <sys/select.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>

#include "packet.h"
#include "conn.h"
#include "comm.h"
#include "protocol.h"
#include "utils.h"

#define APPPORT 6001
#define PEERPORT 7001
#define MONPORT 8000

void reconfigRoutes(config_t * oldcfg, config_t * newcfg, fd_set * fdset)
{
	struct sockaddr_in addr_in;
	int i;
	for (i = 0; i < oldcfg->n; i++) {
		FD_CLR(oldcfg->socket[i], fdset);
		close(oldcfg->socket[i]);
	}
	for (i = 0; i < newcfg->n; i++) {
		newcfg->socket[i] = createSocket4(SOCK_DGRAM);
		addr_in = setSocket4("127.0.0.1", newcfg->port[i]);
		if (connect
		    (newcfg->socket[i], (struct sockaddr *) &addr_in,
		     sizeof(addr_in)) < 0)
			err(1,
			    "reconfigRoutes(...): connect(port=%d,socketfd=%d)",
			    newcfg->port[i], newcfg->socket[i]);
		else
			FD_SET(newcfg->socket[i], fdset);
	}
}

int main(int argc, char *argv[])
{
	int retsel, maxfd;
	char monAnswer;
	uint32_t i, appPktId, peerPktId, expPktId = 0;
	int monitorSock, appSock, peerSock;
	config_t oldcfg, newcfg, tmpcfg;
	packet_t nackPkt, *appPkt, *peerPkt, *recvQueue = NULL, *sendQueue =
		NULL;
	fd_set infds, allsetinfds, outfds, allsetoutfds;

	memset(&newcfg, 0, sizeof(newcfg));
	FD_ZERO(&allsetinfds);
	FD_ZERO(&allsetoutfds);

	monitorSock = connectToMon(MONPORT);
	FD_SET(monitorSock, &allsetinfds);

	appSock = acceptFromApp(listenFromApp(APPPORT));
	FD_SET(appSock, &allsetinfds);
	FD_SET(appSock, &allsetoutfds);

	peerSock = listenUDP4(PEERPORT);
	FD_SET(peerSock, &allsetinfds);

	maxfd = peerSock;

	while (1) {
		infds = allsetinfds;
		outfds = allsetoutfds;
		retsel = select(maxfd + 1, &infds, &outfds, NULL, NULL);
		if (retsel > 0) {
			if (FD_ISSET(monitorSock, &infds)) {
				monAnswer =
					recvMonitorPkts(monitorSock, &tmpcfg);
				switch (monAnswer) {
				case 'A':
					manageMonAck(&tmpcfg, appPkt,
						     sendQueue);
					break;
				case 'N':
					manageMonNack(&tmpcfg, appPkt,
						      &newcfg);
					break;
				case 'C':
					oldcfg = newcfg;
					newcfg = tmpcfg;
					reconfigRoutes(&oldcfg, &newcfg,
						       &allsetoutfds);
					break;
				}
			}
			if (FD_ISSET(appSock, &infds)) {
				appPkt = (packet_t *)
					malloc(sizeof(packet_t));
				appPkt->numfail = nackPkt.numfail;
				memcpy(&appPkt->failid, &nackPkt.failid,
				       sizeof(nackPkt.failid));
				appPktId = recvVoicePkts(appSock, appPkt);
				sendVoicePkts(selectPath(&newcfg), appPkt);
			}
			if (FD_ISSET(peerSock, &infds)) {
				peerPkt =
					(packet_t *) malloc(sizeof(packet_t));
				peerPktId = recvVoicePkts(peerSock, peerPkt);
				if (FD_ISSET(appSock, &outfds)) {
					expPktId +=
						sendPktsToApp(appSock,
							      peerPkt,
							      recvQueue,
							      expPktId);
					nackPkt.numfail =
						peerPktId - expPktId;
				}
				else
					err(132, "App socket not ready!");
				for (i = expPktId; i < peerPktId; i++) {
					nackPkt.failid[i - expPktId] = i;
				}
				warnx("expPktId=%u", expPktId);
			}
		}
		else {
			err(1, "select()");
		}
	}
	return 127;		/* we mustn't reach this point! */
}
