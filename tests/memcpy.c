#include <stdio.h>
#include "macro.h"
int main(int argc, char *argv[])
{
    packet_t packet;
    char ppacket[PACKET_SIZE];

    printf("&ppacket = %u\n", (int) &ppacket);
    printf("&ppacket + sizeof(packet.id) = %u\n",
           (int) ((char *) &ppacket + sizeof(packet.id)));
    return 0;
}
