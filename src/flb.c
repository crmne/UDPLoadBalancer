#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <sys/select.h>
#include <string.h>

#include "packet.h"
#include "conn.h"
#include "comm.h"
#include "protocol.h"
#include "utils.h"

#define APPPORT 11001
#define PEERPORT 10001

int main(int argc, char *argv[])
{
	int retsel, maxfd;
	uint32_t i, appAnswer, peerPktId, expPktId = 0;
	int appSock, peerSock;
	packet_t nackPkt, *appPkt, *peerPkt, *pktQueue = NULL;
	fd_set infds, allsetinfds, outfds, allsetoutfds;

	FD_ZERO(&allsetinfds);
	FD_ZERO(&allsetoutfds);

	appSock = acceptFromApp(listenFromApp(APPPORT));
	FD_SET(appSock, &allsetinfds);
	FD_SET(appSock, &allsetoutfds);


	peerSock = listenUDP4(PEERPORT);
	FD_SET(peerSock, &allsetinfds);

	maxfd = peerSock;

	while (1) {
		outfds = allsetoutfds;
		infds = allsetinfds;
		retsel = select(maxfd + 1, &infds, &outfds, NULL, NULL);
		if (retsel > 0) {
			if (FD_ISSET(peerSock, &infds)) {
				peerPkt =
					(packet_t *) malloc(sizeof(packet_t));
				peerPktId = recvVoicePkts(peerSock, peerPkt);
				if (FD_ISSET(appSock, &outfds)) {
					expPktId +=
						sendPktsToApp(appSock,
							      peerPkt,
							      pktQueue,
							      expPktId);
					nackPkt.numfail =
						peerPktId - expPktId;
				}
				else
					err(132, "App Socket not ready!");
				for (i = expPktId; i < peerPktId; i++) {
					nackPkt.failid[i - expPktId] = i;
				}
				warnx("expPktId=%u", expPktId);
			}
			if (FD_ISSET(appSock, &infds)) {
				appPkt = (packet_t *)
					malloc(sizeof(packet_t));
				appPkt->numfail = nackPkt.numfail;
				memcpy(&appPkt->failid, &nackPkt.failid,
				       sizeof(nackPkt.failid));
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