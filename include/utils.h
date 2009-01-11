#ifndef __UTILS_H__
#define __UTILS_H__

#include <sys/time.h>

/* I do not want to include stdio.h just for NULL */
#ifndef NULL
#define NULL (void *) 0
#endif
#define MAXCONNECTIONS 1
#define MAXPATHS 3
#define PKTSIZE 100

double calcDelay(struct timeval);

#endif
