#include <stdio.h>
#include "macro.h"
int main(int argc, char *argv[])
{
    packet_t packet;
    char ppacket[PACKET_SIZE];

    printf("&ppacket = %i\n", &ppacket);
    printf("&ppacket + sizeof(packet.id) = %i\n",
	   (char *) &ppacket + sizeof(packet.id));
    return 0;
}
