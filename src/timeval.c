#include <err.h>
#include "macro.h"
uint32_t timeval_diff(const struct timeval * a, const struct timeval * b)
{
    uint32_t r;

    if (timeval_cmp(a, b) < 0) {
        const struct timeval *c;

        c = a;
        a = b;
        b = c;
    }
    r = ((uint32_t) a->tv_sec - (uint32_t) b->tv_sec) * USEC_PER_SEC;
    if (a->tv_usec > b->tv_usec)
        r += ((uint32_t) a->tv_usec - (uint32_t) b->tv_usec);
    else if (a->tv_usec < b->tv_usec)
        r -= ((uint32_t) b->tv_usec - (uint32_t) a->tv_usec);
    return r;
}

int timeval_cmp(const struct timeval *a, const struct timeval *b)
{
    if (a->tv_sec < b->tv_sec)
        return -1;
    if (a->tv_sec > b->tv_sec)
        return 1;
    if (a->tv_usec < b->tv_usec)
        return -1;
    if (a->tv_usec > b->tv_usec)
        return 1;
    return 0;
}

uint32_t timeval_age(const struct timeval * tv)
{
    struct timeval now;

    if (gettimeofday(&now, NULL) != 0)
        err(ERR_GETTIMEOFDAY);
    return timeval_diff(&now, tv);
}
