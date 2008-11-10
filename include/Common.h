#ifndef __COMMON_H__
#define __COMMON_H__

/* start TODO */
#define ACK 1
#define NACK 0
typedef struct
{
	int todo;
} config_t;

/* end TODO */
void configSigHandlers();

int listenFromApp(int);
int connectToMon(int);
#endif
