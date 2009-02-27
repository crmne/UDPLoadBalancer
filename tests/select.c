#include <stdio.h>
#include <err.h>
#include <sys/select.h>
#include "macro.h"
#define HOST "127.0.0.1"
#define APPPORT 6001
#define MONPORT 8000
int main(int argc, char *argv[])
{
    int retsel, fd;
    char monAnswer;
    uint32_t appAnswer;
    int monitorSock, listenSock, appSock;
    config_t tempcfg;
    packet_t prova;
    fd_set infds, allsetinfds;

    appSock = 0;
    FD_ZERO(&allsetinfds);

    monitorSock = connect_mon(HOST, MONPORT);
    FD_SET(monitorSock, &allsetinfds);

    listenSock = listen_app(HOST, APPPORT);
    FD_SET(listenSock, &allsetinfds);

    fd = listenSock;

    while (1) {
        infds = allsetinfds;
        retsel = select(fd + 1, &infds, NULL, NULL, NULL);
        if (retsel > 0) {
            if (FD_ISSET(monitorSock, &infds)) {
                monAnswer = recv_mon(monitorSock, &tempcfg);
            }
            if (FD_ISSET(appSock, &infds)) {
                appAnswer = recv_voice_pkts(appSock, &prova, SOCK_STREAM, NULL);
            }
            if (FD_ISSET(listenSock, &infds)) {
                appSock = accept_app(listenSock);
                FD_SET(appSock, &allsetinfds);
                fd = appSock;
            }
        } else {
            err(1, "select()");
        }
    }
    return 127;                 /* we mustn't reach this point! */
}
