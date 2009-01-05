#include <stdio.h>
#include <err.h>
#include <sys/select.h>
#include "packet.h"
#include "conn.h"
#include "comm.h"
#include "packet.h"
#define APPPORT 6001
#define MONPORT 8000
int main(int argc, char *argv[])
{
	int retsel, fd;
	char monAnswer;
	uint32_t appAnswer;
	int monitorSock, listenSock, appSock;
	config_t oldcfg, newcfg, tempcfg;
	packet_t prova;
	fd_set infds, allsetinfds;


	FD_ZERO(&allsetinfds);

	monitorSock = connectToMon(MONPORT);
	FD_SET(monitorSock, &allsetinfds);

	listenSock = listenFromApp(APPPORT);
	FD_SET(listenSock, &allsetinfds);

	fd = listenSock;

	while (1) {
		infds = allsetinfds;
		retsel = select(fd + 1, &infds, NULL, NULL, NULL);
		if (retsel > 0) {
			if (FD_ISSET(monitorSock, &infds)) {
				monAnswer =
					recvMonitorPkts(monitorSock,
							&tempcfg);
			}
			if (FD_ISSET(appSock, &infds)) {
				appAnswer = recvVoicePkts(appSock, &prova);
			}
			if (FD_ISSET(listenSock, &infds)) {
				appSock = acceptFromApp(listenSock);
				FD_SET(appSock, &allsetinfds);
				fd = appSock;
			}
		}
		else {
			err(1, "select()");
		}
	}
	return 127;		/* we mustn't reach this point! */
}
