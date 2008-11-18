#include <stdio.h>
#include <err.h>
#include <sys/select.h>
#include "Common.h"
#define APPPORT 11001
#define PEERPORT 10001
int main(int argc, char *argv[])
{
	int retsel, maxfd;
	uint32_t appAnswer;
	int appSock, peerSock;
	packet_t appPkt, peerPkt;
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
				recvVoicePkts(peerSock, &peerPkt);
				sendVoicePkts(appSock, &peerPkt);
			}
			if (FD_ISSET(appSock, &infds)) {
				appAnswer = recvVoicePkts(appSock, &appPkt);
				sendVoicePkts(peerSock, &appPkt);
			}
		}
		else {
			err(1, "select()");
		}
	}
	return 127;		/* we mustn't reach this point! */
}
