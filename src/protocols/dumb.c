#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "macro.h"
unsigned int path = 0;
struct sockaddr_in *select_path(config_t * config, struct sockaddr_in *to,
                                uint32_t average_delay)
{
    to->sin_family = AF_INET;
    to->sin_addr.s_addr = inet_addr(HOST);
    if (config->n != 0) {
        if (average_delay > (MAX_DELAY * 8 / 15))
            path = (path + 1) % config->n;
        to->sin_port = htons(config->port[path]);

    } else {
        warnx("No available routes!");
        to->sin_port = 0;
    }
    return to;
}

void manage_ack(packet_t * lastSent)
{
    free(lastSent);
}

void manage_nack(int socketfd, packet_t * lastSent, config_t * config,
                 uint32_t average_delay)
{
    struct sockaddr_in to;
    send_voice_pkts(socketfd, lastSent, SOCK_DGRAM,
                    select_path(config, &to, average_delay));
}
