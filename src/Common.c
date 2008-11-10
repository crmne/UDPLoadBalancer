#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <err.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define MAXCONNECTIONS 10
void flushQueues()
{
	fprintf(stderr, "flushQueues() is a stub!\n");
}

void signalHandler(int signum)
{
	fprintf(stderr, "Received signal %d\n", signum);
	flushQueues();
	exit(0);
}

/** Configure signal handlers */
void configSigHandlers()
{
	struct sigaction act;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO;	/* maybe not needed */
	act.sa_sigaction = signalHandler;

	if (sigaction(SIGINT, &act, NULL) < 0)
		err(1, "failed to set SIGINT handler");
	if (sigaction(SIGTERM, &act, NULL) < 0)
		err(1, "failed to set SIGTERM handler");

}

struct sockaddr_in setSocket4(const char *addr, int port)
{
	struct sockaddr_in sock;
	memset(&sock, 0, sizeof(sock));
	sock.sin_family = AF_INET;
	sock.sin_addr.s_addr = inet_addr(addr);
	if (sock.sin_addr.s_addr == INADDR_NONE)
		err(1, "setSocket4(%s,%d): invalid address", addr, port);
	sock.sin_port = htons(port);
	return sock;
}

int createSocket4(int type)
{
	int on;
	int socketfd = socket(AF_INET, type, 0);
	if (socketfd < 0)
		err(1, "createSocket4(%d): socket()", type);

	/* avoid EADDRINUSE */
	on = 1;
	if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) <
	    0)
		err(1, "createSocket4(%d): setsockopt(%d)", type, socketfd);

	return socketfd;
}

int listenFromApp(int port)
{
	struct sockaddr_in sock;
	int socketfd;
#ifdef DEBUG
	printf("Setting up server for App on port %d... ", port);
#endif
	socketfd = createSocket4(SOCK_STREAM);
	sock = setSocket4("0.0.0.0", port);
	if (bind(socketfd, (struct sockaddr *) &sock, sizeof(sock)) < 0)
		err(1, "listenFromApp(%d): bind(%d)", port, socketfd);
	if (listen(socketfd, MAXCONNECTIONS) < 0)
		err(1, "listenFromApp(%d): listen(%d)", port, socketfd);
#ifdef DEBUG
	printf("ok\n");
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
	local = setSocket4("0.0.0.0", 0);
	if (bind(socketfd, (struct sockaddr *) &local, sizeof(local)) < 0)
		err(1, "connectToMon(%d): bind(%d)", port, socketfd);

	serv = setSocket4("127.0.0.1", port);
	if (connect(socketfd, (struct sockaddr *) &serv, sizeof(serv)) < 0)
		err(1, "connectToMon(%d): connect(%d)", port, socketfd);
#ifdef DEBUG
	printf("ok\n");
	fflush(stdout);
#endif
	return socketfd;
}
