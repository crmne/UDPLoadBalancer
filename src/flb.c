#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <sys/select.h>
#include <string.h>

#include "macro.h"

#define APP_PORT 11001
#define PEER_PORT 10001
#define PATH_PORT 9001
#define FIRST_PACKET_ID 0

uint32_t last_expired_packet(struct timeval * conv_start,
			     const uint32_t first)
{
    uint32_t delay = timeval_age(conv_start);
    uint32_t unsigned_max = -MAX_DELAY / PACKET_RATE;
    uint32_t packet_id = first + ((delay - MAX_DELAY) / PACKET_RATE);

    if (packet_id >= unsigned_max)
	return 0;
    else
	return packet_id;
}

struct timeval *call_start_time(struct timeval *conv_start,
				const struct timeval *packet_time,
				const uint32_t packet_id,
				const uint32_t first)
{
    uint32_t added_delay = (packet_id - first) * PACKET_RATE;
    memcpy(conv_start, packet_time, sizeof(struct timeval));
    return timeval_sub(conv_start, added_delay);
}

struct timeval *next_timeout(struct timeval *conv_start,
			     struct timeval *timeout)
{
    uint32_t age = timeval_age(conv_start);

    if (age < MAX_TIME) {
	age = MAX_TIME - age;
    } else {
	age -= MAX_TIME;
	age = PACKET_RATE - (age % PACKET_RATE);
    }


    return timeval_store(timeout, age);
}
int main(int argc, char *argv[])
{
    int socks, maxfd;
    uint32_t appAnswer, peerPktId, expired_pkt_id, expPktId =
	FIRST_PACKET_ID;
    int appSock, peerSock, pathSock;
    struct timeval *timeout = NULL, tick, conv_start;
    packet_t *appPkt = NULL, *peerPkt = NULL, *recvQueue = NULL;
    fd_set infds, allsetinfds;

    expired_pkt_id = expPktId - 1;

    timeval_store(&tick, PACKET_RATE);

    FD_ZERO(&allsetinfds);

    appSock = acceptFromApp(listenFromApp("127.0.0.1", APP_PORT));
    FD_SET(appSock, &allsetinfds);


    peerSock = listenUDP4("127.0.0.1", PEER_PORT);
    FD_SET(peerSock, &allsetinfds);

    pathSock = connectUDP4("127.0.0.1", PATH_PORT);

    maxfd = peerSock;

    while (1) {
	infds = allsetinfds;
	socks = select(maxfd + 1, &infds, NULL, NULL, timeout);
	if (timeout != NULL) {


	    expired_pkt_id =
		last_expired_packet(&conv_start, FIRST_PACKET_ID);

	    if (recvQueue != NULL && recvQueue->id == expired_pkt_id) {
		packet_t *exppkt = getFirstInQ(&recvQueue);

		sendVoicePkts(appSock, exppkt);
		free(exppkt);
	    }
	    if (expPktId <= expired_pkt_id)
		expPktId = expired_pkt_id + 1;

	    next_timeout(&conv_start, timeout);
#ifdef DEBUG
	    warnx
		("Next timeout in %u usecs! Expired packet is %u, Call time %u",
		 timeval_load(timeout), expired_pkt_id,
		 timeval_age(&conv_start));
#endif
	}
	if (socks > 0) {
	    if (FD_ISSET(peerSock, &infds)) {
		peerPkt = (packet_t *) malloc(sizeof(packet_t));
		peerPktId = recvVoicePkts(peerSock, peerPkt);
		if (timeout == NULL) {
		    uint32_t delay =
			timeval_age(call_start_time
				    (&conv_start, &peerPkt->time,
				     peerPktId, FIRST_PACKET_ID));

		    warnx("Call started %u usecs ago!", delay);
		    timeout =
			(struct timeval *) malloc(sizeof(struct timeval));
		    next_timeout(&conv_start, timeout);
		    expired_pkt_id =
			last_expired_packet(&conv_start, FIRST_PACKET_ID);
		    expPktId = peerPktId;
#ifdef DEBUG
		    warnx
			("Next timeout is due in %u usecs! Expired packet is %u",
			 timeval_load(timeout), expired_pkt_id);
#endif
		}
		/*      if (timeval_age(&peerPkt->time) < MAX_DELAY) */
		sendPktsToApp(appSock, peerPkt, &recvQueue, &expPktId);
/*		else {
		    warnx("Packet %u is too late! Rejecting..", peerPktId);
		    free(peerPkt);
		}*/

#ifdef DEBUG
		warnx("expPktId=%d", expPktId);
#endif
	    }
	    if (FD_ISSET(appSock, &infds)) {
		appPkt = (packet_t *) malloc(sizeof(packet_t));
		appAnswer = recvVoicePkts(appSock, appPkt);
		/* TODO */
		sendVoicePkts(pathSock, appPkt);
	    }
	} else if (socks < 0) {
	    err(1, "select()");
	}
    }
    return 127;			/* we mustn't reach this point! */
}
