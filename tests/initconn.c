#include "conn.h"
int main(int argc, char *argv[])
{
    listenFromApp("127.0.0.1", 6001);
    connectToMon("127.0.0.1", 8000);
    return 0;
}
