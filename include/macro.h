#ifndef __MACRO_H__
#define __MACRO_H__

/* I do not want to include stdio.h just for NULL */
#ifndef NULL
#define NULL (void *) 0
#endif
#define HOST "127.0.0.1"
#define MAX_CONNS 1
#define MAX_PATHS 3
#define MAX_DELAY 150000
#define PACKET_RATE 40000
#define PACKET_SIZE 100

#include "error.h"
#include "packet.h"
#include "timeval.h"
#include "conn.h"
#include "comm.h"
#include "queue.h"

#endif
