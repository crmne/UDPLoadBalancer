/* Appmobile.c  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "Util.h"

/* #define VICDEBUG */

#define PKTSIZE 100

fd_set rdset, all;
int fds[2];
int maxfd;
int numricevuti=0;
int numpersi=0;
int numritardi=0;
int printed=0;
uint32_t idmsg=0;
uint32_t idlastrecv=-1;
FILE *f=NULL;

void sig_close(int signo)
{
	if(f!=NULL) 
	{
		fclose(f);
		f=NULL;
	}
	fprintf(stderr,"\nnumricevuti %d  -  numpersi %d  -  numritardi %d\n",
			numricevuti, numpersi, numritardi );
	fflush(stderr);
	exit(888);
}

void EExit(int num)
{
	sig_close(num);
}

void *scheduler(void *p)
{
	struct timeval timeout; char ch=1; int ris;

	for(;;)
	{
		do {
			rdset=all;
			timeout.tv_sec=0;
			timeout.tv_usec=40000;
			ris=select(1,NULL,NULL,NULL,&timeout);
		} while( (ris<0) && (errno==EINTR) );
		if(ris<0) {
			perror("thread scheduler - select failed: TERMINO ");
			sleep(1);
			EExit(1);
		}
		do {
			ris=send(fds[1],&ch,1, MSG_NOSIGNAL);
		} while( (ris<0)&&(errno==EINTR));
		if(ris<0) {
			perror("thread scheduler - select failed: TERMINO ");
			sleep(1);
			EExit(1);
		}
	}
	pthread_exit(NULL);
	return(NULL);
}

void save_delay(FILE *f, uint32_t *buf)
{
	struct timeval sent, now, tempoimpiegato;
	uint32_t idmsg;
	long int msec;

	if(f==NULL) return;
	idmsg=buf[0];
	memcpy( (char*)&sent, (char*)&(buf[1]), sizeof(struct timeval) );
	gettimeofday(&now,NULL);
	/*
	fprintf( f,	"\nsent: %lu : %ld sec %ld usec\n", idmsg, sent.tv_sec, sent.tv_usec );
	fprintf( f,	"now:  %lu : %ld sec %ld usec\n", idmsg, now.tv_sec, now.tv_usec );
	*/
	tempoimpiegato=differenza(now,sent);
	msec=(tempoimpiegato.tv_sec*1000)+(tempoimpiegato.tv_usec/1000);
	/*
	fprintf(	f, "tempoimpiegato: %lu : %ld sec %ld usec\n\n", 
			idmsg, tempoimpiegato.tv_sec, tempoimpiegato.tv_usec );
	*/
	if(msec>150)
	{
		numritardi++;
		printf("%u : delay msec %ld  TEMPO SUPERATO\n", idmsg, msec);
	}
	else
		printf("%u : delay msec %ld\n", idmsg, msec);
	fprintf(f,"%u %ld\n", idmsg, msec);
	fflush(f);

	/*
	if(msec>150)
	{
		fprintf( f,	"\nsent: %lu : %ld sec %ld usec\n", idmsg, sent.tv_sec, sent.tv_usec );
		fprintf( f,	"now:  %lu : %ld sec %ld usec\n", idmsg, now.tv_sec, now.tv_usec );
		fprintf(	f, "tempoimpiegato: %lu : %ld sec %ld usec\n\n", 
				idmsg, tempoimpiegato.tv_sec, tempoimpiegato.tv_usec );
		fprintf(f, "TEMPO SUPERATO - TERMINO\n");
		fflush(f);
		exit(888);
	}
	*/
}


int send_udp(uint32_t socketfd, char *buf, uint32_t len, uint16_t port_number_local, char *IPaddr, uint16_t port_number_dest)
{
	int ris;
	struct sockaddr_in To;
	int addr_size;

	/* assign our destination address */
	memset( (char*)&To,0,sizeof(To));
	To.sin_family		=	AF_INET;
	To.sin_addr.s_addr  =	inet_addr(IPaddr);
	To.sin_port			=	htons(port_number_dest);

	addr_size = sizeof(struct sockaddr_in);
	/* send to the address */
	ris = sendto(socketfd, buf, len , MSG_NOSIGNAL, (struct sockaddr*)&To, addr_size);
	if (ris < 0) {
		printf ("sendto() failed, Error: %d \"%s\"\n", errno,strerror(errno));
		return(0);
	}
	return(1);
}



#define PARAMETRIDEFAULT "./Appmobile.exe 6001"
void usage(void) 
{  printf ( "usage:  ./Appmobile.exe REMOTEPORT\n"
			"esempio: "  PARAMETRIDEFAULT "\n");
}

int main(int argc, char *argv[])
{
	uint16_t portLBmobile;
	int ris, LBmobilefd;
	pthread_t th;
	int primoricevuto=0;

	printf ("uso i parametri di default \n%s\n", PARAMETRIDEFAULT );
	portLBmobile = 6001;

	if ((signal (SIGHUP, sig_close)) == SIG_ERR) { perror("signal (SIGHUP) failed: "); EExit(2); }
	if ((signal (SIGINT, sig_close)) == SIG_ERR) { perror("signal (SIGINT) failed: "); EExit(2); }
	if ((signal (SIGTERM, sig_close)) == SIG_ERR) { perror("signal (SIGTERM) failed: "); EExit(2); }

	init_random();
	ris=socketpair(AF_UNIX,SOCK_STREAM,0,fds);
	if (ris < 0) {	perror("socketpair fds0 failed: ");	EExit(1); }

	/* mi connetto al LBmobile */
	ris=TCP_setup_connection(&LBmobilefd, "127.0.0.1", portLBmobile,  300000, 300000, 1);
	if(!ris) {	printf ("TCP_setup_connection() failed\n"); EExit(1); }
	f=fopen("delaymobile.txt","wt");
	if(f==NULL) { perror("fopen failed"); EExit(1); }

	FD_ZERO(&all);
	FD_SET(LBmobilefd,&all);
	maxfd=LBmobilefd;
	FD_SET(fds[0],&all);
	if(maxfd<fds[0]) maxfd=fds[0];

	ris = pthread_create (&th, NULL, scheduler, NULL );
	if (ris){
		printf("ERROR; return code from pthread_create() is %d\n",ris);
		EExit(-1);
	}

	for(;;)
	{
		struct timeval timeout;

		do {
			rdset=all;
			timeout.tv_sec=10;
			timeout.tv_usec=0;
			ris=select(maxfd+1,&rdset,NULL,NULL,&timeout);
			/* ris=select(maxfd+1,&rdset,NULL,NULL,&timeout); */
		} while( (ris<0) && (errno==EINTR) );
		if(ris<0) {
			perror("select failed: ");
			EExit(1);
		}

		/* se arriva qualcosa dalla connessione TCP con LBmobile, leggo!!!! */
		if( FD_ISSET(LBmobilefd,&rdset) )
		{
			uint32_t buf[PKTSIZE], idletto; int ris;

#ifdef VICDEBUG
			fprintf(stderr,"in arrivo qualcosa dalla connessione TCP del LBmobile:\n");
#endif
			/* ris=recv(LBmobilefd,(char*)buf,PKTSIZE,MSG_DONTWAIT); */
			ris=Readn(LBmobilefd,(char*)buf,PKTSIZE);
			if(ris!=PKTSIZE) { fprintf(stderr,"recv from LBmobile failed, received %d: ", ris); EExit(9); }
			idletto=buf[0];
			printf("ricevuto pkt id %u\n",idletto);

			if(primoricevuto==0) {
				primoricevuto=1;
				idlastrecv=idletto;
				numricevuti=1;
			} 
			else if(idletto<=idlastrecv) {
				fprintf(stderr,"ERRORE, RICEVUTO PKT FUORI ORDINE: letto %u precedente %u  - TERMINO\n",idletto, idlastrecv);
				EExit(999);
			} 
			else {
				if(idlastrecv+1 < idletto) {
					numpersi += (idletto-(idlastrecv+1));
					fprintf(stderr,"PERSO \n");
				} 
				idlastrecv=idletto;
				numricevuti++;
			}
			/* salvo delay pkt */
			save_delay(f,buf);
		}
		else
		{
			if( FD_ISSET(fds[0],&rdset) )
			{
				char ch;
				uint32_t buf[PKTSIZE];
				struct timeval sent;

				do {
					ris=recv(fds[0],&ch,1,0);
				} while( (ris<0) && (errno==EINTR) );
				if(ris<0) {
					perror("Appmobile - recv from scheduler failed: ");
					sleep(1);
					EExit(1);
				}
				/* spedisco i pkt */
				memset((char*)buf,0,PKTSIZE);
				buf[0]=idmsg;
				gettimeofday(&sent,NULL);
				memcpy( (char*)&(buf[1]), (char*)&sent, sizeof(struct timeval) );

				ris=Sendn(LBmobilefd, (char*)buf, PKTSIZE  );
				if(ris!=PKTSIZE) {
					fprintf(stderr,"Appmobile - Sendn failed   ris %d  TERMINO\n", ris);
					sleep(1);
					EExit(1);
				}
				fprintf(stderr,"pkt %u sent %dB\n", idmsg, ris);
				idmsg++;
			}
		}

	} /* fine for ;; */
	return(0);
}

