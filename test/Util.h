/* Util.h  per BASSO, MEDIOBASSO e MEDIOALTO  */

#ifndef __UTIL_H__
#define __UTIL_H__

#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
*/

/*	numero di ping in prova Latenza 
	e numero di byte spediti ogni volta
	in prova Banda */
#define LENVET 100L 

/*	numero di ripetizioni in prova Banda */
#define NUMVOLTE 100L

#define NUMBYTES 20L

#define MAXNUMCONNECTIONS 3

#ifndef MSG_NOSIGNAL
#ifdef SO_NOSIGPIPE
#define MSG_NOSIGNAL SO_NOSIGPIPE
#else
#define MSG_NOSIGNAL 0
#endif
#endif

#ifdef __UTIL_C__
#define UTIL_EXTERN
#else
#define UTIL_EXTERN extern
#endif

typedef void* (*ptr_thread_routine)(void *);

UTIL_EXTERN int		UDP_setup_socket_bound( int32_t *psocketfd, uint16_t numero_porta_locale, int dimSndBuf, int dimRcvBuf);
UTIL_EXTERN int		TCP_setup_connection(int *pserverfd, char *string_IP_remote_address, int port_number_remote,
										 int dimSndBuf, int dimRcvBuf, int TcpNoDelay);
UTIL_EXTERN int		TCP_setup_socket_listening(int *plistenfd, int numero_porta_locale,
											   int dimSndBuf, int dimRcvBuf, int TcpNoDelay);
UTIL_EXTERN ssize_t Writen (int fd, const void *buf, size_t n);
UTIL_EXTERN int		Sendn (int fd, const void *buf, int n);
UTIL_EXTERN int		Readn(int fd, char *ptr, int nbytes);
UTIL_EXTERN int		SetsockoptReuseAddr(int s);
UTIL_EXTERN int		GetsockoptReuseAddr(int s, int *pFlag);
UTIL_EXTERN int		SetsockoptTCPNODELAY(int s, int booleano);
UTIL_EXTERN int		GetsockoptTCPNODELAY(int s, int *pboolean);
UTIL_EXTERN int		SetsockoptSndBuf(int s, int numbytes);
UTIL_EXTERN int		GetsockoptSndBuf(int s, int *pnumbytes);
UTIL_EXTERN int		SetsockoptRcvBuf(int s, int numbytes);
UTIL_EXTERN int		GetsockoptRcvBuf(int s, int *pnumbytes);
UTIL_EXTERN int		SetNoBlocking(int s);
UTIL_EXTERN int		SetBlocking(int s);
UTIL_EXTERN int		IsBlocking(int s, int *pIsBlocking);
UTIL_EXTERN void	init_random(void);
UTIL_EXTERN unsigned int	inizializza(char *buf, int len);
UTIL_EXTERN unsigned int	sommavet(char *buf, int len);
UTIL_EXTERN unsigned int	stampavet(char *buf, int len);
UTIL_EXTERN void*	thread_For_Write (int *psocketfd);
UTIL_EXTERN void*	thread_For_Read (int *psocketfd);
UTIL_EXTERN int		normalizza( struct timeval *t );
UTIL_EXTERN int		somma(struct timeval tmr,struct timeval ist,struct timeval *delay);
UTIL_EXTERN struct	timeval differenza(struct timeval dopo,struct timeval prima);
UTIL_EXTERN int		minore(struct timeval *a, struct timeval *b);
UTIL_EXTERN int minoreouguale(struct timeval *a, struct timeval *b);
UTIL_EXTERN void	stampa_timeval(char *str, struct timeval tv);



#endif   /*  __UTIL_H__  */ 


