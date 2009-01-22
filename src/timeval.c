#include <err.h>
#include "timeval.h"
uint32_t timeval_diff(const struct timeval *a, const struct timeval *b)
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

struct timeval *timeval_sub(struct timeval *tv, uint32_t v)
{
    unsigned long secs;

    secs = (unsigned long) (v / USEC_PER_SEC);
    tv->tv_sec -= (time_t) secs;
    v -= ((uint32_t) secs) * USEC_PER_SEC;

    if (tv->tv_usec >= (suseconds_t) v)
	tv->tv_usec -= (suseconds_t) v;
    else {
	tv->tv_sec--;
	tv->tv_usec += (suseconds_t) (USEC_PER_SEC - v);
    }
    return tv;
}

struct timeval *timeval_add(struct timeval *tv, uint32_t v)
{
    unsigned long secs;

    secs = (unsigned long) (v / USEC_PER_SEC);
    tv->tv_sec -= (time_t) secs;
    v -= ((uint32_t) secs) * USEC_PER_SEC;

    tv->tv_usec += (suseconds_t) v;

    while ((unsigned) tv->tv_usec >= USEC_PER_SEC) {
	tv->tv_sec++;
	tv->tv_usec -= (suseconds_t) USEC_PER_SEC;
    }
    return tv;

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

struct timeval *timeval_store(struct timeval *tv, uint32_t v)
{
    tv->tv_sec = (time_t) (v / USEC_PER_SEC);
    tv->tv_usec = (suseconds_t) (v % USEC_PER_SEC);
    return tv;
}

uint32_t timeval_load(const struct timeval * tv)
{
    return (uint32_t) tv->tv_sec * USEC_PER_SEC + (uint32_t) tv->tv_usec;
}

uint32_t timeval_age(const struct timeval * tv)
{
    struct timeval now;

    if (gettimeofday(&now, NULL) != 0)
	err(1, "gettimeofday()");
    return timeval_diff(&now, tv);
}
