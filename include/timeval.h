#ifndef __TIMEVAL_H__
#define __TIMEVAL_H__

#include <sys/time.h>
#include <stdint.h>

#define USEC_PER_SEC ((uint32_t) 1000000)
#define USEC_PER_MSEC ((uint32_t) 1000)

uint32_t timeval_diff(const struct timeval *, const struct timeval *);
struct timeval *timeval_sub(struct timeval *, uint32_t);
struct timeval *timeval_add(struct timeval *, uint32_t);
int timeval_cmp(const struct timeval *, const struct timeval *);
struct timeval *timeval_store(struct timeval *, uint32_t);
uint32_t timeval_load(const struct timeval *);
uint32_t timeval_age(const struct timeval *);

#endif
