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
void flushQueues()
{
	fprintf(stderr, "flushQueues() is a stub!\n");
	/* TODO: close fds? */
}

int signalHandler(int signum)
{
	fprintf(stderr, "\nReceived signal %d\n", signum);
	flushQueues();
	exit(0);
}

/** Configure signal handlers */
void configSigHandlers()
{
	struct sigaction act;
#ifdef DEBUG
	printf("Setting up signal handlers... ");
#endif
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO;	/* maybe not needed */
	act.sa_sigaction = signalHandler;

	if (sigaction(SIGINT, &act, NULL) < 0)
		err(1, "failed to set SIGINT handler");
	if (sigaction(SIGTERM, &act, NULL) < 0)
		err(1, "failed to set SIGTERM handler");
#ifdef DEBUG
	printf("OK\n");
	fflush(stdout);
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
#ifdef DEBUG
	printf("Setting up server for App on port %d... ", port);
#endif
	socketfd = createSocket4(SOCK_STREAM);
	sock = setSocket4("127.0.0.1", port);
	if (bind(socketfd, (struct sockaddr *) &sock, sizeof(sock)) < 0)
		err(1, "listenFromApp(port=%d): bind(socketfd=%d)", port,
		    socketfd);
	if (listen(socketfd, MAXCONNECTIONS) < 0)
		err(1, "listenFromApp(port=%d): listen(socketfd=%d)", port,
		    socketfd);
#ifdef DEBUG
	printf("OK\n");
	fflush(stdout);
#endif
	return socketfd;
}

int connectToMon(int port)
{
	struct sockaddr_in local, serv;
	int socketfd;
#ifdef DEBUG
	printf("Connecting to Monitor on port %d... ", port);
#endif
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
	printf("OK\n");
	fflush(stdout);
#endif
	return socketfd;
}

int acceptFromApp(int socketfd)
{
	int newsocketfd;
	unsigned int len;
	struct sockaddr_in sock;
#ifdef DEBUG
	printf("App wants to connect to us... ");
#endif
	len = sizeof(sock);
	memset(&sock, 0, len);
	newsocketfd = accept(socketfd, (struct sockaddr *) &sock, &len);
	if (newsocketfd < 0)
		err(1, "acceptFromApp(socketfd=%d): accept()", socketfd);
#ifdef DEBUG
	printf("Connected!\n");
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
#ifdef DEBUG
	printf("Received new ");
#endif
	n = read(socketfd, &answer, sizeof(answer));
	if (n != sizeof(answer))
		err(1, "recvMonitorPkts(socketfd=%d,...): read(answer)",
		    socketfd);
	newconfig->type = answer;
	n = read(socketfd, &newconfig->n, sizeof(newconfig->n));
	if (n != sizeof(newconfig->n))
		err(1, "recvMonitorPkts(socketfd=%d,...): read(n)", socketfd);
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

uint32_t recvVoicePkts(int socketfd, packet_t * packet)
{
	int n;
	n = read(socketfd, &packet->id, sizeof(packet->id));
	if (n != sizeof(packet->id))
		err(1, "recvVoicePkts(socketfd=%d,...): read(packet->id)",
		    socketfd);
	n = read(socketfd, &packet->time, sizeof(packet->time));
	if (n != sizeof(packet->time))
		err(1, "recvVoicePkts(socketfd=%d,...): read(packet->time)",
		    socketfd);
	n = read(socketfd, &packet->data, sizeof(packet->data));
	if (n != PKTSIZE - sizeof(packet->id) - sizeof(packet->time))
		err(1, "recvVoicePkts(socketfd=%d,...): read(packet->data)",
		    socketfd);
#ifdef DEBUG
	printf("Received voice packet %u\n", packet->id);	/* TODO: from who? */
	fflush(stdout);
#endif
	return packet->id;
}

void sendVoicePkts(int socketfd, packet_t * packet)
{
	int n;
	n = write(socketfd, &packet, sizeof(*packet));
	if (n != PKTSIZE)
		err(1, "sendVoicePkts(socketfd=%d,...): write(packet)%d",
		    socketfd, sizeof(*packet));
#ifdef DEBUG
	printf("Sending voice packet %u to Peer\n", packet->id);
	fflush(stdout);
#endif

}

void reconfigRoutes(config_t * oldcfg, config_t * newcfg, fd_set * fdset,
		    int *maxfd)
{
	struct sockaddr_in addr_in;
	int i;
	for (i = 0; i < oldcfg->n; i++) {
		close(oldcfg->socket[i]);
		FD_CLR(oldcfg->socket[i], fdset);
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
		FD_SET(newcfg->socket[i], fdset);
	}
	if (newcfg->n > 0)
		*maxfd = newcfg->socket[i];
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

void doSomething()
{
	fprintf(stderr, "...\n");
}
