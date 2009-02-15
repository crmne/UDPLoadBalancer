#include <err.h>
#include <stdlib.h>
#include <string.h>
#include "macro.h"

int select_path(config_t * config)
{
    return config->socket[0];
}

void manage_ack(config_t * ack, packet_t * lastSent)
{
    free(lastSent);
}

void manage_nack(config_t * nack, packet_t * lastSent, config_t * config)
{
    send_voice_pkts(select_path(config), lastSent);
}

unsigned int pa_cpy_to_pp(char *pp, struct packet_additions_t *pa)
{
    int i;
    unsigned int n = 0;
    if (pa->n != 0) {
        memcpy(pp, &pa->n, sizeof(pa->n));
        n = sizeof(pa->n);
        for (i = 0; i < pa->n; i++) {
            memcpy((char *) pp + n, &pa->port[i], sizeof(pa->port[i]));
            n += sizeof(pa->port[i]);
        }
    }
    return n;
}

unsigned int pa_cpy_from_pp(struct packet_additions_t *pa, char *pp)
{
    int i;
    unsigned int n = 0;
    if (pa->n != 0) {
        memcpy(&pa->n, pp, sizeof(pa->n));
        n = sizeof(pa->n);
        for (i = 0; i < pa->n; i++) {
            memcpy(&pa->port[i], (char *) pp + n, sizeof(pa->port[i]));
            n += sizeof(pa->port[i]);
        }
    }
    return n;
}
