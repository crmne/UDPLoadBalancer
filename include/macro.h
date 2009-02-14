#ifndef __MACRO_H__
#define __MACRO_H__

/* I do not want to include stdio.h just for NULL */
#ifndef NULL
#define NULL (void *) 0
#endif
#define MAX_CONNS 1
#define MAX_PATHS 3
#define MAX_DELAY 150000
#define PACKET_RATE 40000
#define PACKET_SIZE 100
#define MAX_TIME MAX_DELAY - 5000

#include "packet.h"
#include "timeval.h"
#include "conn.h"
#include "comm.h"
#include "protocol.h"
#include "queue.h"

#endif
