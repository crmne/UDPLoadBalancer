#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <err.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Common.h"
#include "Queues.h"
int signalHandler(int signum)
{
	errx(signum, "\nReceived signal %d\n", signum);
}

/** Configure signal handlers */
void configSigHandlers()
{
#ifdef SIGHANDLER
	struct sigaction act;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO;	/* maybe not needed */
	act.sa_sigaction = signalHandler;

	if (sigaction(SIGINT, &act, NULL) < 0)
		err(1, "failed to set SIGINT handler");
	if (sigaction(SIGTERM, &act, NULL) < 0)
		err(1, "failed to set SIGTERM handler");
	if (sigaction(SIGHUP, &act, NULL) < 0)
		err(1, "failed to set SIGINT handler");
#ifdef DEBUG
	printf("Signal handlers setup OK\n");
	fflush(stdout);
#endif
#endif
}

struct sockaddr_in setSocket4(const char *addr, int port)
{
	struct sockaddr_in sock;
	memset(&sock, 0, sizeof(sock));
	sock.sin_family = AF_INET;
	if (inet_aton(addr, &sock.sin_addr) == 0)
		err(1, "setSocket4(addr=%s,port=%d): inet_aton()", addr,
		    port);
	sock.sin_port = htons(port);
	return sock;
}

int createSocket4(int type)
{
	int on;
	int socketfd = socket(AF_INET, type, 0);
	if (socketfd < 0)
		err(1, "createSocket4(type=%d): socket()", type);

	/* avoid EADDRINUSE */
	on = 1;
	if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) <
	    0)
		err(1, "createSocket4(type=%d): setsockopt(socketfd=%d)",
		    type, socketfd);

	return socketfd;
}

/** Sets up a listening socket
 * @param port which we want to listen
 * @return file descriptor of created socket
 */
int listenFromApp(int port)
{
	struct sockaddr_in sock;
	int socketfd;
	socketfd = createSocket4(SOCK_STREAM);
	sock = setSocket4("127.0.0.1", port);
	if (bind(socketfd, (struct sockaddr *) &sock, sizeof(sock)) < 0)
		err(1, "listenFromApp(port=%d): bind(socketfd=%d)", port,
		    socketfd);
	if (listen(socketfd, MAXCONNECTIONS) < 0)
		err(1, "listenFromApp(port=%d): listen(socketfd=%d)", port,
		    socketfd);
#ifdef DEBUG
	printf("Waiting for App on port %d...\n", port);
	fflush(stdout);
#endif
	return socketfd;
}

int connectToMon(int port)
{
	struct sockaddr_in local, serv;
	int socketfd;
	socketfd = createSocket4(SOCK_STREAM);
	local = setSocket4("127.0.0.1", 0);
	if (bind(socketfd, (struct sockaddr *) &local, sizeof(local)) < 0)
		err(1, "connectToMon(port=%d): bind(socketfd=%d)", port,
		    socketfd);

	serv = setSocket4("127.0.0.1", port);
	if (connect(socketfd, (struct sockaddr *) &serv, sizeof(serv)) < 0)
		err(1, "connectToMon(port=%d): connect(socketfd=%d)", port,
		    socketfd);
#ifdef DEBUG
	printf("Connected to Monitor on port %d\n", port);
	fflush(stdout);
#endif
	return socketfd;
}

int acceptFromApp(int socketfd)
{
	int newsocketfd;
	unsigned int len;
	struct sockaddr_in sock;
	len = sizeof(sock);
	memset(&sock, 0, len);
	newsocketfd = accept(socketfd, (struct sockaddr *) &sock, &len);
	if (newsocketfd < 0)
		err(1, "acceptFromApp(socketfd=%d): accept()", socketfd);
#ifdef DEBUG
	printf("Connected to App\n");
	fflush(stdout);
#endif
	return newsocketfd;

}

/** Receive packets from monitor
 * @param socketfd
 * @param buffer
 * @return C for config, A for ack, N for nack
 */
char recvMonitorPkts(int socketfd, config_t * newconfig)
{
	int n, i;
	char answer;
	n = read(socketfd, &answer, sizeof(answer));
	if (n == 0)
		errx(2, "Nothing received, maybe Monitor is down? Exiting.");
	if (n != sizeof(answer))
		err(1, "recvMonitorPkts(socketfd=%d,...): read(answer)",
		    socketfd);
	newconfig->type = answer;
	n = read(socketfd, &newconfig->n, sizeof(newconfig->n));
	if (n != sizeof(newconfig->n))
		err(1, "recvMonitorPkts(socketfd=%d,...): read(n)", socketfd);
#ifdef DEBUG
	printf("Received new ");
#endif
	switch (answer) {
	case 'C':
#ifdef DEBUG
		printf("configuration: ");
#endif
		for (i = 0; i < newconfig->n; i++) {
			n = read(socketfd, &newconfig->port[i],
				 sizeof(newconfig->port[i]));
			if (n != sizeof(newconfig->port[i]))
				err(1,
				    "recvMonitorPkts(socketfd=%d,...): read(port[%d])",
				    socketfd, i);
#ifdef DEBUG
			printf("%u ", newconfig->port[i]);
#endif
		}

		break;
	case 'A':
#ifdef DEBUG
		printf("ack ");
#endif
		break;
	case 'N':
		/* read read */
#ifdef DEBUG
		printf("nack ");
#endif
		break;
	default:
		errx(1,
		     "recvMonitorPkts(socketfd=%d,...): read(): Invalid packet",
		     socketfd);
	}
#ifdef DEBUG
	printf("from Monitor\n");
	fflush(stdout);
#endif
	return answer;
}

double calcDelay(struct timeval older)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	return ((double) (now.tv_sec - older.tv_sec) +
		(double) (now.tv_usec - older.tv_usec) / 1000000.0) * 1000;
}

uint32_t recvVoicePkts(int socketfd, packet_t * packet)
{
	int n;
	char ppacket[PKTSIZE];
	n = read(socketfd, &ppacket, sizeof(ppacket));
	if (n == 0)
		errx(2,
		     "Nothing received, maybe the other end is down? Exiting.");
	if (n != sizeof(ppacket))
		err(1,
		    "recvVoicePkts(socketfd=%d,...): read(packet)=%d",
		    socketfd, n);
	memcpy(&packet->id, &ppacket, sizeof(packet->id));
	memcpy(&packet->time, (char *) &ppacket + sizeof(packet->id),
	       sizeof(packet->time));
	memcpy(&packet->data,
	       (char *) &ppacket + sizeof(packet->id) + sizeof(packet->time),
	       sizeof(packet->data));
	packet->next = NULL;
#ifdef DEBUG
	printf("Received voice packet %u, delay = %f msec\n", packet->id, calcDelay(packet->time));	/* TODO: from who? */
	fflush(stdout);
#endif
	return packet->id;
}

void sendVoicePkts(int socketfd, packet_t * packet)
{
	int n;
	char ppacket[PKTSIZE];
	memcpy(&ppacket, &packet->id, sizeof(packet->id));
	memcpy((char *) &ppacket + sizeof(packet->id), &packet->time,
	       sizeof(packet->time));
	memcpy((char *) &ppacket + sizeof(packet->id) + sizeof(packet->time),
	       &packet->data, sizeof(packet->data));
	n = write(socketfd, &ppacket, sizeof(ppacket));
	if (n != sizeof(ppacket))
		err(1, "sendVoicePkts(socketfd=%d,...): write(packet)%lu",
		    socketfd, sizeof(ppacket));
#ifdef DEBUG
	printf("Sending voice packet %u, delay = %f msec\n", packet->id,
	       calcDelay(packet->time));
	fflush(stdout);
#endif

}

void reconfigRoutes(config_t * oldcfg, config_t * newcfg)
{
	struct sockaddr_in addr_in;
	int i;
	for (i = 0; i < oldcfg->n; i++) {
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
	}
}

int listenUDP4(int port)
{
	struct sockaddr_in addr_in;
	int socketfd = createSocket4(SOCK_DGRAM);
	addr_in = setSocket4("127.0.0.1", port);
	if (bind(socketfd, (struct sockaddr *) &addr_in, sizeof(addr_in)) < 0)
		err(1, "listenFromApp(port=%d): bind(socketfd=%d)", port,
		    socketfd);
	return socketfd;
}

uint32_t sendPktsToApp(int appSock, packet_t * peerPkt, packet_t * pktQueue,
		       uint32_t expPktId)
{
	packet_t *first;
	uint32_t i = 0;
	if (peerPkt->id == expPktId) {
		sendVoicePkts(appSock, peerPkt);
		free(peerPkt);
		i = 1;
		if (pktQueue != NULL) {
			while ((first = getFirstInQ(&pktQueue)) != NULL
			       && first->id == expPktId + i) {
				sendVoicePkts(appSock, first);
				free(first);
				i++;
			}
		}
		/*sendAckToPeer(); */
	}
	else {
		insertInQ(&pktQueue, peerPkt);
		/*sendNackToPeer(); */
	}
	return i;
}

void doSomething()
{
	fprintf(stderr, "...\n");
}
