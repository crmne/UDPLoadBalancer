#include "Common.h"
int main(int argc, char *argv[])
{
	configSigHandlers();
	listenFromApp(6001);
	connectToMon(8000);
	return 0;
}
