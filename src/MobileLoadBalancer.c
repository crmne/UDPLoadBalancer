#include <stdio.h>
#include <err.h>
#include <sys/select.h>
#include <string.h>
#include "Common.h"
#define APPPORT 6001
#define PEERPORT 7001
#define MONPORT 8000
int main(int argc, char *argv[])
{
	int retsel, maxfd;
	char monAnswer;
	uint32_t appPktId, peerPktId;
	int monitorSock, appSock, peerSock;
	config_t oldcfg, newcfg, tmpcfg;
	packet_t appPkt, peerPkt;
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
					doSomething();
					break;
				case 'N':
					doSomething();
					break;
				case 'C':
					oldcfg = newcfg;
					newcfg = tmpcfg;
					reconfigRoutes(&oldcfg, &newcfg);
					break;
				}
			}
			if (FD_ISSET(appSock, &infds)) {
				appPktId = recvVoicePkts(appSock, &appPkt);
				sendVoicePkts(selectPath(&newcfg), &appPkt);
			}
			if (FD_ISSET(peerSock, &infds)) {
				peerPktId = recvVoicePkts(peerSock, &peerPkt);
				/* TODO: sort packets */
				sendVoicePkts(appSock, &peerPkt);
			}
		}
		else {
			err(1, "select()");
		}
	}
	return 127;		/* we mustn't reach this point! */
}
