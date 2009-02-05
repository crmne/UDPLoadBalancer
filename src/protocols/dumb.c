#include <err.h>
#include <stdlib.h>
#include <string.h>
#include "macro.h"

struct packet_additions_t next_pa;

int select_path(config_t * config)
{
    return config->socket[0];
}

void manage_ack(config_t * ack, packet_t * lastSent)
{
    if (lastSent->id == ack->n) {
        free(lastSent);
    } else {
        warnx("ACK %u, LASTSENT %u", ack->n, lastSent->id);
        /* q_insert(&sendQueue, lastSent); ?? */
    }
}

void manage_nack(config_t * nack, packet_t * lastSent, config_t * config)
{
    if (lastSent->id == nack->n) {
        send_voice_pkts(select_path(config), lastSent);
    } else {
        warnx("NACK %u, LASTSENT %u", nack->n, lastSent->id);
    }
}

void pa_cpy_to_pp(char *pp, struct packet_additions_t *pa)
{
    memcpy(pp, &next_pa, sizeof(next_pa));
}

void pa_cpy_from_pp(struct packet_additions_t *pa, char *pp)
{
    memcpy(pa, pp, sizeof(pa));
    if (pa->ack_prev_p == 'N')
        warnx("PROTO: NACK");
}

void is_not_exp_pkt(packet_t * packet, uint32_t exp_id)
{
    next_pa.ack_prev_p = 'N';
}
