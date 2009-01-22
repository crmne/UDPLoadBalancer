#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <sys/select.h>
#include <string.h>
#include <unistd.h>

#include "macro.h"

#define APP_PORT 6001
#define PEER_PORT 7001
#define MON_PORT 8000
#define FIRST_PACKET_ID 9999

void reconfigRoutes(config_t * oldcfg, config_t * newcfg)
{
    int i;

    for (i = 0; i < oldcfg->n; i++) {
	close(oldcfg->socket[i]);
    }
    for (i = 0; i < newcfg->n; i++) {
	newcfg->socket[i] = connectUDP4("127.0.0.1", newcfg->port[i]);
    }
}

int main(int argc, char *argv[])
{
    int socks, maxfd;
    char monAnswer;
    uint32_t appPktId, peerPktId, expired_pkt_id, expPktId =
	FIRST_PACKET_ID;
    int monitorSock, appSock, peerSock, pathSock;
    config_t oldcfg, newcfg, tmpcfg;
    struct timeval *timeout = NULL, tick;
    packet_t *appPkt = NULL, *peerPkt = NULL, *recvQueue =
	NULL, *sendQueue = NULL;
    fd_set infds, allsetinfds;

    expired_pkt_id = expPktId - 1;

    timeval_store(&tick, PACKET_RATE);

    memset(&newcfg, 0, sizeof(newcfg));
    FD_ZERO(&allsetinfds);

    monitorSock = connectToMon("127.0.0.1", MON_PORT);
    FD_SET(monitorSock, &allsetinfds);

    appSock = acceptFromApp(listenFromApp("127.0.0.1", APP_PORT));
    FD_SET(appSock, &allsetinfds);

    peerSock = listenUDP4("127.0.0.1", PEER_PORT);
    FD_SET(peerSock, &allsetinfds);

    maxfd = peerSock;

    while (1) {
	infds = allsetinfds;
	socks = select(maxfd + 1, &infds, NULL, NULL, timeout);
	if (socks == 0) {
	    if (timeval_cmp(&tick, timeout) != 0)
		memcpy(timeout, &tick, sizeof(tick));

	    expired_pkt_id++;

	    if (expPktId <= expired_pkt_id)
		expPktId = expired_pkt_id + 1;
#ifdef DEBUG
	    warnx("Tick: Packet %u has expired.. expected is now %u",
		  expired_pkt_id, expPktId);
#endif

	} else if (socks > 0) {
	    if (FD_ISSET(monitorSock, &infds)) {
		monAnswer = recvMonitorPkts(monitorSock, &tmpcfg);
		switch (monAnswer) {
		case 'A':
		    manageMonAck(&tmpcfg, appPkt, sendQueue);
		    break;
		case 'N':
		    manageMonNack(&tmpcfg, appPkt, &newcfg);
		    break;
		case 'C':
		    oldcfg = newcfg;
		    newcfg = tmpcfg;
		    reconfigRoutes(&oldcfg, &newcfg);
		    break;
		default:
		    errx(1, "Monitor packet not understood!");
		}
	    }
	    if (FD_ISSET(appSock, &infds)) {
		appPkt = (packet_t *) malloc(sizeof(packet_t));
		appPktId = recvVoicePkts(appSock, appPkt);
		pathSock = selectPath(&newcfg);
		if (appPkt->id > 5)
		    sendVoicePkts(pathSock, appPkt);
	    }
	    if (FD_ISSET(peerSock, &infds)) {
		peerPkt = (packet_t *) malloc(sizeof(packet_t));
		peerPktId = recvVoicePkts(peerSock, peerPkt);
		if (timeout == NULL) {
		    /* calcolare l'inizio della chiamata */
		    uint32_t delay = timeval_age(&peerPkt->time);

		    delay += (peerPktId - FIRST_PACKET_ID) * PACKET_RATE;
		    expPktId = peerPktId;
		    warnx("Call started %u usecs ago!", delay);
		    /* calcolare il tempo che manca per far scadere il
		       primo pacchetto */
		    timeout =
			(struct timeval *) malloc(sizeof(struct timeval));
		    if (delay < MAX_TIME) {
			delay = MAX_TIME - delay;
		    } else {
			delay -= MAX_TIME;
			expired_pkt_id =
			    FIRST_PACKET_ID + (delay / PACKET_RATE);
			delay = PACKET_RATE - (delay % PACKET_RATE);

		    }
		    timeval_store(timeout, delay);
#ifdef DEBUG
		    warnx
			("Next timeout is due in %u usecs! Expired packet is %u",
			 delay, expired_pkt_id);
#endif

		}
/*		if (timeval_age(&peerPkt->time) < MAX_DELAY)*/
		sendPktsToApp(appSock, peerPkt, &recvQueue, &expPktId);
/*		else {
		    warnx("Packet %u is too late! Rejecting..", peerPktId);
		    free(peerPkt);
		}*/
#ifdef DEBUG
		warnx("expPktId=%d", expPktId);
#endif
	    }
	} else {
	    err(1, "select()");
	}
    }
    return 127;			/* we mustn't reach this point! */
}
