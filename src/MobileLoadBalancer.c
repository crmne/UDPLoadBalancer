#include "Common.h"
void configSignals() {

}

socket connectToMon() {

}

socket connectToApp() {

}

int main(int argc, char* argv[]) {
	socket monitorSock, appSock;
	fd_set fdsetw, fdsetr;

	configSignals();
	monitorSock = connectToMon();
	appSock = connectToApp();

	for (;;) {
		fd_set = checksocks(monitorSock,appSock);
		if (FD_ISSET(monitor)) {
			checkconf();
		}
		if (FD_ISSET(app)) {
			sendPkts(recvAppPkts());
		}
		if (FD_ISSET()){
			recvPkts();
		}
	}
}
