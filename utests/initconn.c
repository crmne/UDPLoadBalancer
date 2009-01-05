#include "conn.h"
int main(int argc, char *argv[])
{
	listenFromApp(6001);
	connectToMon(8000);
	return 0;
}
