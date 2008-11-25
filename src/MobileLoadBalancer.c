#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <sys/select.h>
#include <string.h>
#include "Common.h"
#include "Proto.h"

#define APPPORT 6001
#define PEERPORT 7001
#define MONPORT 8000

int main(int argc, char *argv[])
{
	int retsel, maxfd;
	char monAnswer;
	uint32_t i, appPktId, peerPktId, expPktId = 0;
	int monitorSock, appSock, peerSock;
	config_t oldcfg, newcfg, tmpcfg;
	packet_t nackPkt, *appPkt, *peerPkt, *recvQueue = NULL, *sendQueue =
		NULL;
	fd_set infds, allsetinfds;

	configSigHandlers();
	memset(&newcfg, 0, sizeof(newcfg));
	FD_ZERO(&allsetinfds);

	monitorSock = connectToMon(MONPORT);
	FD_SET(monitorSock, &allsetinfds);

	appSock = acceptFromApp(listenFromApp(APPPORT));
	FD_SET(appSock, &allsetinfds);

	peerSock = listenUDP4(PEERPORT);
	FD_SET(peerSock, &allsetinfds);

	maxfd = peerSock;

	while (1) {
		infds = allsetinfds;
		retsel = select(maxfd + 1, &infds, NULL, NULL, NULL);
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
					reconfigRoutes(&oldcfg, &newcfg);
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
				expPktId +=
					sendPktsToApp(appSock, peerPkt,
						      recvQueue, expPktId);
				nackPkt.numfail = peerPktId - expPktId;
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
