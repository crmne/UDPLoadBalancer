void configSignals() {

}

socket connectToMon() {

}

socket connectToApp() {

}

int main(int argc, char* argv[]) {
	socket appSock;
	fd_set fdsetw, fdsetr;

	configSignals();
	appSock = connectToApp();

	for (;;) {
		/*fd_set = checksocks(appSock);*/
		if (FD_ISSET()){
			recvPkts();
		}
		if (FD_ISSET(app)) {
			sendPkts(recvAppPkts(6001));
		}
	}
}
