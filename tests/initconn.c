#include "conn.h"
int main(int argc, char *argv[])
{
    listen_app("127.0.0.1", 6001);
    connect_mon("127.0.0.1", 8000);
    return 0;
}
