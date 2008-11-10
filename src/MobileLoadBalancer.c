#include <stdio.h>
#include <err.h>
#include <sys/select.h>
#include "Common.h"
#define APPPORT 6001
#define MONPORT 8000
config_t recvNewConf(int socketfd)
{

}

int main(int argc, char *argv[])
{
	int retsel, fd;
	config_t currentConf, oldConf;
	int monitorData;
	int monitorSock, appSock;
	fd_set infds, allsetinfds;

	configSigHandlers();

	FD_ZERO(&allsetinfds);

	monitorSock = connectToMon(MONPORT);
	FD_SET(monitorSock, &allsetinfds);

	appSock = listenFromApp(APPPORT);
	FD_SET(appSock, &allsetinfds);

	while (1) {
		infds = allsetinfds;
		retsel = select(fd + 1, &infds, NULL, NULL, NULL);
		if (retsel > 0) {
			if (FD_ISSET(monitorSock, &infds)) {
				monitorData = recvPkts(monitorSock);

				if (monitorData == NACK)
					doSomething();
				else if (monitorData == ACK)
					doSomething();

				else {
					oldConf = currentConf;
					currentConf = monitorData;
					if (currentConf != oldConf)
						reconfigureConns();
				}

			}
			if (FD_ISSET(appSock, &infds)) {
				sendVoicePkts(recvPkts(APPPORT), currentConf);
			}
			if (FD_ISSET(otherside, &infds)) {
				recvVoicePkts(currentConf);
			}
		}
		else {
			err(1, "select()");
		}
	}
	return 127;		/* we mustn't reach this point! */
}
