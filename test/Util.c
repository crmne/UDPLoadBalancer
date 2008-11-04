/* Util.c */

#define __UTIL_C__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>

#include "Util.h"

/*
typedef void* (*ptr_thread_routine)(void *);

int		TCP_setup_connection(int *pserverfd, char *string_IP_remote_address, int port_number_remote);
int		TCP_setup_socket_listening(int *plistenfd, int numero_porta_locale);
ssize_t Writen (int fd, const void *buf, size_t n);
int		Readn(int fd, char *ptr, int nbytes);
int		SetsockoptReuseAddr(int s);
int		GetsockoptReuseAddr(int s, int *pFlag);
int		SetNoBlocking(int s);
int		SetBlocking(int s);
int		IsBlocking(int s, int *pIsBlocking);
void	init_random(void);
int		inizializza(char *buf, int len);
int		sommavet(char *buf, int len);
int		stampavet(char *buf, int len);
void*	thread_For_Write (int *psocketfd);
void*	thread_For_Read (int *psocketfd);
*/

#define SEC_IN_MCSEC 1000000L

#ifdef TIOCOUTQ
int	FilledSndBufferSpace(int s, int *pnum)
{
	if (ioctl (s,TIOCOUTQ,pnum)==0) 
		return(1);
	else {
		perror("fcntl TIOCOUTQ failed: ");
		exit(0);
		return(-1);
	}
}
#endif


int UDP_setup_socket_bound( int32_t *psocketfd, uint16_t numero_porta_locale, int dimSndBuf, int dimRcvBuf)
{
	int ris;
	struct sockaddr_in Local;
	
	*psocketfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (*psocketfd < 0)
	{	
		printf ("socket() failed, Err: %d \"%s\"\n", errno,strerror(errno));
		return(0);
	}

	/* avoid EADDRINUSE error on bind() */
	ris = SetsockoptReuseAddr(*psocketfd);
	if (!ris)
	{
		printf ("SetsockoptReuseAddr() failed, Err: %d \"%s\"\n", errno,strerror(errno));
		return(0);
	}

	if(dimSndBuf>0) {
		int n;
		ris=SetsockoptSndBuf(*psocketfd,dimSndBuf);
		if (!ris)  exit(5);
		ris=GetsockoptSndBuf(*psocketfd,&n);
		if (!ris)  exit(5);
		/* else printf ("SndBuf socketfd %d\n", n); */
	}
	if(dimRcvBuf>0) {
		int n;
		ris=SetsockoptRcvBuf(*psocketfd,dimRcvBuf);
		if (!ris)  exit(5);
		ris=GetsockoptRcvBuf(*psocketfd,&n);
		if (!ris)  exit(5);
		/* else printf ("RcvBuf socketfd %d\n", n); */
	}

	/*name the socket */
	memset ( &Local, 0, sizeof(Local) );
	Local.sin_family= AF_INET;
	/* specifico l'indirizzo IP attraverso cui voglio ricevere la connessione */
	Local.sin_addr.s_addr = htonl(INADDR_ANY);
	Local.sin_port	      = htons(numero_porta_locale);

	ris = bind(*psocketfd, (struct sockaddr*) &Local, sizeof(Local));
	if (ris<0)
	{	
		printf ("bind() failed, Err: %d \"%s\"\n",errno,strerror(errno));
		fflush(stderr);
		return(0);
	}
	return(1);
}


int TCP_setup_connection(int *pserverfd, char *string_IP_remote_address, int port_number_remote,
						 int dimSndBuf, int dimRcvBuf, int TcpNoDelay)
{
	struct sockaddr_in Local, Serv;
	int ris;

	*pserverfd = socket(AF_INET, SOCK_STREAM, 0);
	if (*pserverfd<0) {
		printf ("socket() failed, Err: %d \"%s\"\n", errno,strerror(errno));
		fflush(stdout);
		return(0);
	}

	if(dimSndBuf>0) {
		int n;
		ris=SetsockoptSndBuf(*pserverfd,dimSndBuf);
		if (!ris)  exit(5);
		ris=GetsockoptSndBuf(*pserverfd,&n);
		if (!ris)  exit(5);
		else printf ("SndBuf connectedfd %d\n", n);
	}
	if(dimRcvBuf>0) {
		int n;
		ris=SetsockoptRcvBuf(*pserverfd,dimRcvBuf);
		if (!ris)  exit(5);
		ris=GetsockoptRcvBuf(*pserverfd,&n);
		if (!ris)  exit(5);
		else printf ("RcvBuf connectedfd %d\n", n);
	}
	{
		int n;
		ris=SetsockoptTCPNODELAY(*pserverfd,TcpNoDelay);
		if (!ris)  exit(5);
		ris=GetsockoptTCPNODELAY(*pserverfd,&n);
		if (!ris)  exit(5);
		else printf ("TCPNODELAY connectedfd %d\n", n);
	}

	/* name the socket */
	memset ( &Local, 0, sizeof(Local) );
	Local.sin_family		=	AF_INET;
	Local.sin_addr.s_addr	=	htonl(INADDR_ANY);         /* wildcard */
	Local.sin_port			=	htons(0);

	ris = bind(*pserverfd, (struct sockaddr*) &Local, sizeof(Local));
	if (ris<0)  {
	    printf ("bind() failed, Err: %d \"%s\"\n",errno,strerror(errno));
		return(0);
	}

	/* assign our destination address */
	memset ( &Serv, 0, sizeof(Serv) );
	Serv.sin_family	 =	AF_INET;
	Serv.sin_addr.s_addr  =	inet_addr(string_IP_remote_address);
	Serv.sin_port		 =	htons(port_number_remote);

	printf ("connecting to %s %d\n", string_IP_remote_address, port_number_remote);
	fflush(stdout);
	do {
		ris = connect(*pserverfd, (struct sockaddr*) &Serv, sizeof(Serv));
	} while( (ris<0)&&(errno==EINTR));

	if (ris<0)  {
		printf ("connect() failed, Err: %d \"%s\"\n",errno,strerror(errno));
		fflush(stdout);
		return(0);
	}
	printf ("connected to %s %d\n", string_IP_remote_address, port_number_remote);
	fflush(stdout);
	return(1);

}

int TCP_setup_socket_listening(int *plistenfd, int numero_porta_locale,
							   int dimSndBuf, int dimRcvBuf, int TcpNoDelay)
{
	int ris;
	struct sockaddr_in Local;
	
	*plistenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (*plistenfd < 0)
	{	
		printf ("socket() failed, Err: %d \"%s\"\n", errno,strerror(errno));
		return(0);
	}

	/* avoid EADDRINUSE error on bind() */
	ris = SetsockoptReuseAddr(*plistenfd);
	if (!ris)
	{
		printf ("SetsockoptReuseAddr() failed, Err: %d \"%s\"\n", errno,strerror(errno));
		return(0);
	}

	if(dimSndBuf>0) {
		int n;
		ris=SetsockoptSndBuf(*plistenfd,dimSndBuf);
		if (!ris)  exit(5);
		ris=GetsockoptSndBuf(*plistenfd,&n);
		if (!ris)  exit(5);
		/* else printf ("SndBuf listenfd %d\n", n); */
	}
	if(dimRcvBuf>0) {
		int n;
		ris=SetsockoptRcvBuf(*plistenfd,dimRcvBuf);
		if (!ris)  exit(5);
		ris=GetsockoptRcvBuf(*plistenfd,&n);
		if (!ris)  exit(5);
		/* else printf ("RcvBuf listenfd %d\n", n); */
	}
	{
		int n;
		ris=SetsockoptTCPNODELAY(*plistenfd,TcpNoDelay);
		if (!ris)  exit(5);
		ris=GetsockoptTCPNODELAY(*plistenfd,&n);
		if (!ris)  exit(5);
		/* else printf ("TCPNODELAY listenfd %d\n", n); */
	}

	/*name the socket */
	memset ( &Local, 0, sizeof(Local) );
	Local.sin_family= AF_INET;
	/* specifico l'indirizzo IP attraverso cui voglio ricevere la connessione */
	Local.sin_addr.s_addr = htonl(INADDR_ANY);
	Local.sin_port	      = htons(numero_porta_locale);

	ris = bind(*plistenfd, (struct sockaddr*) &Local, sizeof(Local));
	if (ris<0)
	{	
		printf ("bind() failed, Err: %d \"%s\"\n",errno,strerror(errno));
		fflush(stderr);
		return(0);
	}

	/* enable accepting of connection  */
	ris = listen(*plistenfd, 100 );
	if (ris<0)
	{	
		printf ("listen() failed, Err: %d \"%s\"\n",errno,strerror(errno));
		exit(1);
	}
	return(1);
}


ssize_t  Writen (int fd, const void *buf, size_t n)
{   
	size_t	nleft;     
	ssize_t  nwritten;  
	char	*ptr;

	ptr = (void *)buf;
	nleft = n;
	while (nleft > 0) 
	{
		if ( (nwritten = send(fd, ptr, nleft,0)) < 0) {
			if (errno == EINTR)	nwritten = 0;   /* and call write() again*/
			else	{
				int myerrno;
				char msg[2000];
				myerrno=errno;
				sprintf(msg,"write: errore in lettura [nwritten %d] :",nwritten);
				errno=myerrno;
				perror(msg);
				fflush(stderr);
				return(-1);       /* error */
			}
		}
		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}

int  Sendn (int fd, const void *buf, int n)
{   
	size_t	nleft;     
	ssize_t  nwritten;  
	char	*ptr;

	ptr = (void *)buf;
	nleft = n;
	while (nleft > 0) 
	{
		if ( (nwritten = send(fd, ptr, nleft, MSG_NOSIGNAL) ) < 0) {
			if (errno == EINTR)	nwritten = 0;   /* and call write() again*/
			else	{
				int myerrno;
				char msg[2000];
				myerrno=errno;
				sprintf(msg,"send failed: [nwritten %d] :",nwritten);
				errno=myerrno;
				perror(msg);
				fflush(stderr);
				return(-1);       /* error */
			}
		}
		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}

int Readn(int fd, char *ptr, int nbytes)
{
	int nleft,nread;

	nleft=nbytes;
	while(nleft>0)
	{
		do {
			nread=read(fd,ptr,nleft);
		} while ( (nread<0) && (errno==EINTR) );
		if(nread<0)
		{	/* errore */
			char msg[2000];
			sprintf(msg,"Readn: errore in lettura [result %d] :",nread);
			perror(msg);
			return(-1);
		}
		else
		{
			if(nread==0) {
				return(0);
				break;
			}
		}

		nleft -= nread;
		ptr   += nread;
	}
return(nbytes);
}

void init_random(void)
{
	unsigned int seed;
	srandom( (seed=(unsigned int)getpid()) );
	printf("seed %d RAND_MAX %d\n", seed, RAND_MAX);
	fflush(stdout);
}

unsigned int inizializza(char *buf, int len)
{
	unsigned int sum=0;
	int i;

	for(i=0;i<len;i++) 
	{
		buf[i] = '0'+(random()%10);
		sum += buf[i] - '0';
	}
	return(sum);
}

unsigned int sommavet(char *buf, int len)
{
	unsigned int sum=0;
	int i;

	for(i=0;i<len;i++) 
		sum += buf[i] - '0';
	return(sum);
}

unsigned int stampavet(char *buf, int len)
{
	unsigned int sum=0;
	int i;

	printf("\n");
	for(i=0;i<len;i++)
	{
		printf(" %d ", buf[i] );
		sum += (buf[i] - '0');
	}
	printf("   somma %u \n", sum );
	return(sum);
}

int	SetsockoptReuseAddr(int s)
{
	int OptVal, ris;

	/* avoid EADDRINUSE error on bind() */
	OptVal = 1;
	ris = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&OptVal, sizeof(OptVal));
	if (ris != 0 )  {
		printf ("setsockopt() SO_REUSEADDR failed, Err: %d \"%s\"\n", errno,strerror(errno));
		return(0);
	}
	else
		return(1);
}

int	GetsockoptReuseAddr(int s, int *pFlag)
{
	int OptVal, ris;
	unsigned int OptLen;

	ris = getsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&OptVal, &OptLen );
	if (ris != 0 )  {
		printf ("getsockopt() SO_REUSEADDR failed, Err: %d \"%s\"\n", errno,strerror(errno));
		fflush(stdout);
		return(0);
	}
	else {
		*pFlag=OptVal;
		return(1);
	}
}

int	SetsockoptTCPNODELAY(int s, int booleano)
{
	int OptVal, ris;

	if(booleano) OptVal = 1;
	else	 OptVal = 0;
	ris = setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char *)&OptVal, sizeof(OptVal));
	if (ris != 0 )  {
		printf ("setsockopt() TCP_NODELAY failed, Err: %d \"%s\"\n", errno,strerror(errno));
		return(0);
	}
	else
		return(1);
}

int	GetsockoptTCPNODELAY(int s, int *pboolean)
{
	unsigned int OptLen;
	int ris;

	OptLen=sizeof(int);
	ris = getsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char *)pboolean, &OptLen );
	if (ris != 0 )  {
		printf ("getsockopt() TCP_NODELAY failed, Err: %d \"%s\"\n", errno,strerror(errno));
		return(0);
	}
	else
		return(1);
}

int	SetsockoptSndBuf(int s, int numbytes)
{
	int ris;

	ris = setsockopt(s, SOL_SOCKET, SO_SNDBUF, (char *)&numbytes, sizeof(int));
	if (ris != 0 )  {
		printf ("setsockopt() SO_SNDBUF failed, Err: %d \"%s\"\n", errno,strerror(errno));
		return(0);
	}
	else
		return(1);
}

int	GetsockoptSndBuf(int s, int *pnumbytes)
{
	unsigned int OptLen;
	int ris;

	OptLen=sizeof(int);
	ris = getsockopt(s, SOL_SOCKET, SO_SNDBUF, (char *)pnumbytes, &OptLen);
	if (ris != 0 )  {
		printf ("getsockopt() SO_SNDBUF failed, Err: %d \"%s\"\n", errno,strerror(errno));
		return(0);
	}
	else
		return(1);
}

int	SetsockoptRcvBuf(int s, int numbytes)
{
	int ris;

	ris = setsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *)&numbytes, sizeof(int));
	if (ris != 0 )  {
		printf ("setsockopt() SO_RCVBUF failed, Err: %d \"%s\"\n", errno,strerror(errno));
		return(0);
	}
	else
		return(1);
}

int	GetsockoptRcvBuf(int s, int *pnumbytes)
{
	unsigned int OptLen;
	int ris;

	OptLen=sizeof(int);
	ris = getsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *)pnumbytes, &OptLen);
	if (ris != 0 )  {
		printf ("getsockopt() SO_RCVBUF failed, Err: %d \"%s\"\n", errno,strerror(errno));
		return(0);
	}
	else
		return(1);
}

int	SetNoBlocking(int s)
{
	int flags;

	if ( (flags=fcntl(s,F_GETFL,0)) <0 ) {
		printf ("fcntl(F_GETFL) failed, Err: %d \"%s\"\n", errno,strerror(errno));
		return(0);
	}
	flags |= O_NONBLOCK; 
	if ( fcntl(s,F_SETFL,flags) <0 ) {
		printf ("fcntl(F_SETFL) failed, Err: %d \"%s\"\n", errno,strerror(errno));
		return(0);
	}
	return(1);
}

int	SetBlocking(int s)
{
	int flags;

	if ( (flags=fcntl(s,F_GETFL,0)) <0 ) {
		printf ("fcntl(F_GETFL) failed, Err: %d \"%s\"\n", errno,strerror(errno));
		return(0);
	}
	flags &= (~O_NONBLOCK);
	if ( fcntl(s,F_SETFL,flags) <0 ) {
		printf ("fcntl(F_SETFL) failed, Err: %d \"%s\"\n", errno,strerror(errno));
		return(0);
	}
	return(1);
}

int	IsBlocking(int s, int *pIsBlocking)
{
	int flags;

	if ( (flags=fcntl(s,F_GETFL,0)) <0 ) {
		printf ("fcntl(F_GETFL) failed, Err: %d \"%s\"\n", errno,strerror(errno));
		return(0);
	}
	flags &= O_NONBLOCK;
	/* ora flags e' diverso da zero se il socket e' NON bloccante */ 
	if(flags)	*pIsBlocking=0;
	else		*pIsBlocking=1;
	return(1);
}

void *thread_For_Write (int *psocketfd)
{
	int newsocketfd;
	int n, nwrite, K;
	char *buf;
	int *ptr;
	unsigned int sum, sumnet;
#ifdef TIOCOUTQ
	int space;
#endif

	newsocketfd=*psocketfd;
	free(psocketfd);
	psocketfd=NULL;

	/* alloco la struttura in cui restituire il risultato */
	ptr=malloc(sizeof(int));
	if(ptr==NULL) {
		perror("malloc failed: ");
		fflush(stderr);
		pthread_exit (NULL);
		return(NULL);
	}

	buf=malloc(LENVET);
	if(buf==NULL) {
		perror("malloc failed: ");
		fflush(stderr);
		pthread_exit (NULL);
		return(NULL);
	}
	sum=inizializza(buf,LENVET);
	printf("inizializza somma vettore %d\n", sum );
	printf ("pthread thread_For_Write inizia spedizione di %ld bytes\n", NUMVOLTE*(sizeof(int)+LENVET));
	fflush (stdout);
	
	/*
	stampavet(buf,LENVET);
	fflush (stdout);
	*/

	for(K=0;K<NUMVOLTE;K++) 
	{
		printf(".");
		fflush(stdout);

		/* scrittura valore della somma */
		nwrite = sizeof(int);
		sumnet=htonl(sum);
		/*
		printf ("Writen( sum )\n");
		fflush (stdout);
		*/

#ifdef TIOCOUTQ
		n=FilledSndBufferSpace(newsocketfd,&space);
		if (n<0){
			printf("FilledSndBufferSpace failed \n");
			exit ( 3 );
		} 
		else printf("FilledSndBufferSpace %d\n", space);
#endif

		n = Writen (newsocketfd, &sumnet, nwrite );
		if (n != nwrite)
		{
			perror ("Writen() failed \n");
			fflush(stdout);
			exit ( 3 );
		}

#ifdef TIOCOUTQ
		n=FilledSndBufferSpace(newsocketfd,&space);
		if (n<0){
			printf("FilledSndBufferSpace failed \n");
			exit ( 3 );
		} 
		else printf("FilledSndBufferSpace %d\n", space);
#endif

		/* scrittura dati */
		nwrite = LENVET;
		/*
		printf ("Writen()\n");
		fflush (stdout);
		*/
		n = Writen (newsocketfd, buf, nwrite );
		if (n != nwrite)
		{
			perror ("Writen() failed \n");
			fflush(stdout);
			exit ( 3 );
		}
	}
	/* chiusura */
	printf ("terminazione corretta del pthread thread_For_Write\n");
	fflush (stdout);
	*ptr=1;
	pthread_exit ( ptr );  /* valore restituito dal thread */
	return (ptr);
}



void *thread_For_Read (int *psocketfd)
{
	int newsocketfd;
	int n, nread, K;
	char *buf;
	int *ptr;
	unsigned int sum, sumnet, sumricevuta;

	newsocketfd=*psocketfd;
	free(psocketfd);
	psocketfd=NULL;

	/* alloco la struttura in cui restituire il risultato */
	ptr=malloc(sizeof(int));
	if(ptr==NULL) {
		perror("malloc failed: ");
		fflush(stderr);
		pthread_exit (NULL);
		return(NULL);
	}

	buf=malloc(LENVET);
	if(buf==NULL) {
		perror("malloc failed: ");
		fflush(stderr);
		pthread_exit (NULL);
		return(NULL);
	}

	for(K=0;K<NUMVOLTE;K++) 
	{
		printf(".");
		fflush(stdout);

		/* lettura valore della somma */
		nread=sizeof(int);
		/*
		printf ("Readn() sum \n");
		fflush (stdout);
		*/
		n = Readn (newsocketfd, (char*) &sumnet, nread);
		if (n != nread)
		{
			perror ("Readn() failed \n");
			fflush(stdout);
			exit ( 3 );
		}
		sum=ntohl(sumnet);
		/* printf ("valore somma ricevuta %d\n", sum); */

		/* lettura dati */
		nread=LENVET;
		/*
		printf ("Readn()\n");
		fflush (stdout);
		*/
		n = Readn (newsocketfd, buf, nread);
		if (n != nread)
		{
			perror ("Readn() failed \n");
			fflush(stdout);
			exit ( 3 );
		}

	  	sumricevuta=sommavet(buf,LENVET);
		/* printf("valore somma ricevuta %u somma ricalcolata %u\n", sum, sumricevuta ); */

		/*
		stampavet(buf,LENVET);
		fflush(stdout);
		*/

		
		if(sumricevuta!=sum)
		{
			printf ("\033[33;35;1m somma errata: errore in trasmissione dati\n \033[0m ");
			fflush(stdout);
			exit( 3 );
			return (NULL);
		}
		
	}

	/* chiusura */
	printf ("terminazione corretta del pthread thread_For_Read\n");
	fflush (stdout);
	*ptr=1;
	pthread_exit ( ptr );  /* valore restituito dal thread */
	return (ptr);
}

int normalizza( struct timeval *t )
{
	if(t->tv_usec>=SEC_IN_MCSEC)
	{
		t->tv_sec  += ( t->tv_usec/SEC_IN_MCSEC );
		t->tv_usec =  ( t->tv_usec%SEC_IN_MCSEC );
	}
	return(1);
}

int somma(struct timeval tmr,struct timeval ist,struct timeval *delay)
{
	normalizza( &tmr );
	normalizza( &ist );
	
	(*delay).tv_sec = ist.tv_sec + tmr.tv_sec;
	(*delay).tv_usec = ist.tv_usec + tmr.tv_usec;
	normalizza( delay );
	return(1);
}

struct timeval OLDdifferenza(struct timeval dopo,struct timeval prima)
{
	struct timeval diff;
	normalizza( &prima);
	normalizza( &dopo);
	
	diff.tv_sec = dopo.tv_sec - prima.tv_sec;          /* Sottraggo i secondi tra di loro */
	if (diff.tv_sec < 0){
		diff.tv_sec = 0;
		diff.tv_usec = 0;
	}
	else{
		diff.tv_usec = dopo.tv_usec - prima.tv_usec;
		if  (diff.tv_usec < 0){
			if (diff.tv_sec > 0)
			{
				/*
				Devo scalare di uno i secondi e sottrarli ai micro secondi 
				ossia aggiungo 1000000 all'ultima espressione
				*/
				diff.tv_sec =diff.tv_sec - 1;
				diff.tv_usec =(dopo.tv_usec) - prima.tv_usec + SEC_IN_MCSEC;
			}
			else{
				diff.tv_usec = 0;
			}
		}
	}
	return(diff);
}

struct timeval differenza(struct timeval dopo,struct timeval prima)
{
	struct timeval diff;
	normalizza( &prima);
	normalizza( &dopo);
	
	if ( dopo.tv_sec < prima.tv_sec){
		diff.tv_sec = 0;
		diff.tv_usec = 0;
	}
	else {
		diff.tv_sec = dopo.tv_sec - prima.tv_sec;	/* Sottraggo i secondi tra di loro */
		if  (dopo.tv_usec < prima.tv_usec){
			if (diff.tv_sec > 0)
			{
				/*
				Devo scalare di uno i secondi e sottrarli ai micro secondi 
				ossia aggiungo 1000000 all'ultima espressione
				*/
				diff.tv_sec =diff.tv_sec - 1;
				diff.tv_usec =(dopo.tv_usec) - prima.tv_usec + SEC_IN_MCSEC;
			}
			else{
				diff.tv_usec = 0;
			}
		}
		else {
			diff.tv_usec = dopo.tv_usec - prima.tv_usec;
		}
	}
	return(diff);
}

int minore(struct timeval *a, struct timeval *b)
{
	normalizza(a);
	normalizza(b);
	if(a->tv_sec<b->tv_sec) return(1);
	else if (a->tv_usec < b->tv_usec) return(1);
	else return(0);
}

int minoreouguale(struct timeval *a, struct timeval *b)
{
	normalizza(a);
	normalizza(b);
	if(a->tv_sec<b->tv_sec) return(1);
	else if (a->tv_usec <= b->tv_usec) return(1);
	else return(0);
}


void stampa_timeval(char *str, struct timeval tv)
{
	printf("%s %ld.%3ld sec\n", str, tv.tv_sec, tv.tv_usec);
	fflush(stdout);
}
