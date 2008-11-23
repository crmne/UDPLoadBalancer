#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <sys/select.h>
#include "Common.h"
#include "Proto.h"
#define APPPORT 11001
#define PEERPORT 10001
int main(int argc, char *argv[])
{
	int retsel, maxfd;
	uint32_t appAnswer, expPktId = 0;
	int appSock, peerSock;
	packet_t *appPkt, *peerPkt, *pktQueue = NULL;
	fd_set infds, allsetinfds;

	configSigHandlers();
	FD_ZERO(&allsetinfds);

	appSock = acceptFromApp(listenFromApp(APPPORT));
	FD_SET(appSock, &allsetinfds);

	peerSock = listenUDP4(PEERPORT);
	FD_SET(peerSock, &allsetinfds);

	maxfd = peerSock;

	while (1) {
		infds = allsetinfds;
		retsel = select(maxfd + 1, &infds, NULL, NULL, NULL);
		if (retsel > 0) {
			if (FD_ISSET(peerSock, &infds)) {
				peerPkt = (packet_t *) malloc(sizeof(packet_t));
				recvVoicePkts(peerSock, peerPkt);
				expPktId +=
					sendPktsToApp(appSock, peerPkt,
						      pktQueue, expPktId);
				warnx("expPktId=%u", expPktId);
			}
			if (FD_ISSET(appSock, &infds)) {
				appPkt = (packet_t *) malloc(sizeof(packet_t));
				appAnswer = recvVoicePkts(appSock, appPkt);
				/* TODO: find best route */
			}
		}
		else {
			err(1, "select()");
		}
	}
	return 127;		/* we mustn't reach this point! */
}
