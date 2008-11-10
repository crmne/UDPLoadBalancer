#include <stdio.h>
#include <err.h>
#include <sys/select.h>
#include "Common.h"
#define APPPORT 6001
#define MONPORT 8000
int main(int argc, char *argv[])
{
	int retsel, fd;
	char monAnswer;
	int monitorSock, listenSock, appSock;
	config_t oldcfg, newcfg, tempcfg;
	fd_set infds, allsetinfds;

	configSigHandlers();

	FD_ZERO(&allsetinfds);

	monitorSock = connectToMon(MONPORT);
	FD_SET(monitorSock, &allsetinfds);

	listenSock = listenFromApp(APPPORT);
	appSock = acceptFromApp(listenSock);
	FD_SET(appSock, &allsetinfds);

	while (1) {
		fd = appSock;
		infds = allsetinfds;
		retsel = select(fd + 1, &infds, NULL, NULL, NULL);
		if (retsel > 0) {
			if (FD_ISSET(monitorSock, &infds)) {
				fprintf(stderr,
					"Monitor said something...\n");
				monAnswer =
					recvMonitorPkts(monitorSock,
							&tempcfg);
/*
				   if (monitorData == 'N')
				   doSomething();
				   else if (monitorData == 'A')
				   doSomething();

				   else {
				   oldConf = currentConf;
				   currentConf = monitorData;
				   if (currentConf != oldConf)
				   reconfigureConns();
				   }
*/
			}
			if (FD_ISSET(appSock, &infds)) {
				/*fprintf(stderr, "App "); */
				/*sendVoicePkts(recvPkts(APPPORT), currentConf); */
			}
			/*if (FD_ISSET(otherside, &infds)) {
			   recvVoicePkts(currentConf);
			   } */
		}
		else {
			err(1, "select()");
		}
	}
	return 127;		/* we mustn't reach this point! */
}
