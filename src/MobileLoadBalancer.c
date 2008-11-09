#include <stdio.h>
#include <err.h>
#include <sys/select.h>
#include "Common.h"
#define APPPORT 6001
#define MONPORT 8000
config_t recvNewConf(socket_t socket)
{

}

socket_t connectToMon(int port)
{

}

socket_t connectToApp(int port)
{

}

int main(int argc, char *argv[])
{
	int retsel;
	config_t currentConf, oldConf;
	socket_t monitorSock, appSock;
	fd_set infds, allsetinfds;

	configSigHandlers();
	monitorSock = connectToMon(MONPORT);
	appSock = connectToApp(APPPORT);

	while (1) {
		infds = allsetinfds;
		retsel = select(fd + 1, &infds, NULL, NULL, NULL);
		if (retsel > 0) {
			if (FD_ISSET(monitor)) {
				monitorData = recvPkts(monitor);

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
			if (FD_ISSET(app)) {
				sendVoicePkts(recvPkts(APP_PORT),
					      currentConf);
			}
			if (FD_ISSET(otherside)) {
				recvVoicePkts(currentConf);
			}
		}
		else {
			err(1, "select()");
		}
	}
	return 127;		/* we mustn't reach this point! */
}
