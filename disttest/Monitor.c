/* Ritardatore.c  */

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
#include <assert.h>
#include <math.h>

#include "Util.h"

/* #define VICDEBUG */

#define P(X) do { fprintf(stderr,X "\n"); fflush(stderr); } while(0)

#define MAXNUMCONNECTIONS 3
typedef struct {
	int32_t fd_latomobile;
	uint16_t port_number_latomobile;
	int32_t fd_latofixed;
	uint16_t port_number_latofixed;
	int attivo;
	long int sec_istcreazione;
} COPPIAFD;
COPPIAFD coppiafd[MAXNUMCONNECTIONS];

#define CMD_ADDPORT	1
#define CMD_SEND	2

struct structelementolista;
typedef struct structelementolista{
	int cmd;
	int32_t fd;
	uint16_t port_number_local;
	uint16_t port_number_dest;
	uint32_t len;
	char *buf;
	struct timeval timeout;
	struct structelementolista *next;
} ELEMENTOLISTA;
ELEMENTOLISTA *root;

typedef struct {
	char tipoack;
	uint32_t id;
}  __attribute__((packed)) ACK;


#define PERC_ERR 15   /* 15% */

double PERCENTUALE_ERRORE;
int counter_localport_mobileside=0, counter_localport_fixedside=0;
uint16_t	first_local_port_number_mobile_side, first_local_port_number_fixed_side;
fd_set rdset, all;
int listening_monitorfd=-1, monitorfd=-1;
int maxfd;
int numspediti=0;
int numscartati=0;
int printed=0;

void close_coppia(int i);

void sig_print(int signo)
{
	int i;
	if(printed==0)
	{
		printed=1;
		for(i=0;i<MAXNUMCONNECTIONS;i++)
			close_coppia(i);

		if(listening_monitorfd>=0)
		{
			FD_CLR(listening_monitorfd,&all);
			close(listening_monitorfd);
			listening_monitorfd=-1;
		}
		if(monitorfd>=0)
		{
			FD_CLR(monitorfd,&all);
			close(monitorfd);
			monitorfd=-1;
		}

		if(signo==SIGINT)		printf("SIGINT\n");
		else if(signo==SIGHUP)	printf("SIGHUP\n");
		else if(signo==SIGTERM)	printf("SIGTERM\n");
		else					printf("other signo\n");
		printf("\n");
		/*
		for(i=0;i<MAXNUMCONNECTIONS;i++)
		{
			printf("canale %d: sent %d received %d  tot %d\n",
					i, Ricevuti[i][0],Ricevuti[i][1], Ricevuti[i][0]+Ricevuti[i][1] );
		}
		*/
		printf("numspediti %d  numscartati %d\n", numspediti, numscartati );
		fflush(stdout);
	}
	exit(0);
	return;
}

void stampa_fd_set(char *str, fd_set *pset)
{
	int i;
	printf("%s ",str);
	for(i=0;i<100;i++) if(FD_ISSET(i,pset)) printf("%d ", i);
;	printf("\n");
	fflush(stdout);
}

int get_local_port(int socketfd)
{
	int ris;
	struct sockaddr_in Local;
	unsigned int addr_size;

	addr_size=sizeof(Local);
	memset( (char*)&Local,0,sizeof(Local));
	Local.sin_family		=	AF_INET;
	ris=getsockname(socketfd,(struct sockaddr*)&Local,&addr_size);
	if(ris<0) { perror("getsockname() failed: "); return(0); }
	else {
		/*
		fprintf(stderr,"IP %s port %d\n", inet_ntoa(Local.sin_addr), ntohs(Local.sin_port) );
		*/
		return( ntohs(Local.sin_port) );
	}
}

int	send_configurazione(int monitorfd)
{
	int i, ris; uint32_t num, sent=0;
	char ch='C';

	ris=Sendn(monitorfd,&ch,sizeof(ch));
	if(ris!=sizeof(ch)) { fprintf(stderr,"send_configurazione failed: "); exit(9); }
	sent+=ris;

	for(i=0,num=0;i<MAXNUMCONNECTIONS;i++)	if(coppiafd[i].attivo==1) num++;
#ifdef VICDEBUG
	fprintf(stderr,"send_configurazione: num attivi %d\n", num);
#endif
	ris=Sendn(monitorfd,&num,sizeof(num));
	if(ris!=sizeof(num)) { fprintf(stderr,"send_configurazione failed: "); exit(9); }
	sent+=ris;

	for(i=0;i<MAXNUMCONNECTIONS;i++) {
		if(coppiafd[i].attivo==1) {
#ifdef VICDEBUG
			fprintf(stderr,"send_configurazione: porta %d\n", coppiafd[i].port_number_latomobile );
#endif
			ris=Sendn(monitorfd,&(coppiafd[i].port_number_latomobile),sizeof(coppiafd[i].port_number_latomobile));
			if(ris!=sizeof(coppiafd[i].port_number_latomobile)) { fprintf(stderr,"send_configurazione failed: "); exit(9); }
			sent+=ris;
		}
	}
	return(sent);	
}

int check_port(uint16_t port_number_local)
{
	int i;
	
	for(i=0;i<MAXNUMCONNECTIONS;i++)	
	{
		if( (coppiafd[i].attivo==1)  &&
			(
				(
					(coppiafd[i].port_number_latomobile==port_number_local) &&
					(coppiafd[i].fd_latomobile>0) &&
					(coppiafd[i].fd_latofixed>0)
				)

				||
				(
					(coppiafd[i].port_number_latofixed==port_number_local) &&
					(coppiafd[i].fd_latofixed>0) &&
					(coppiafd[i].fd_latomobile>0)
				)
			)
		  )
		  return(1);
	}
	return(0);
}

void aggiungi_in_ordine(ELEMENTOLISTA *p)
{
	ELEMENTOLISTA* *pp=&root;

	if(p==NULL) 
		return;
	while(*pp!=NULL) {
		if( minore( &(p->timeout) , &( (*pp)->timeout) ) ) {
			ELEMENTOLISTA *tmp;
			tmp=*pp;
			*pp=p;
			p->next=tmp;
			tmp=NULL;
			return;
		}
		else
			pp = &( (*pp)->next );
	}
	if(*pp==NULL) {
		*pp=p;
		p->next=NULL;
	}
}

void free_pkt(ELEMENTOLISTA* *proot)
{
	ELEMENTOLISTA *tmp;

	if(proot) {
		if(*proot) {
			tmp=*proot;
			*proot = (*proot)->next;
			if(tmp->buf) {
				free(tmp->buf);
				tmp->buf=NULL;
			}
			free(tmp);
		}
	}
}

void close_coppia(int i)
{
	if(coppiafd[i].attivo==1)
	{
		coppiafd[i].attivo=0;
		if(coppiafd[i].fd_latomobile>=0) 
		{
			FD_CLR(coppiafd[i].fd_latomobile,&all);
			close(coppiafd[i].fd_latomobile);
			coppiafd[i].fd_latomobile=-1;
		}
		if(coppiafd[i].fd_latofixed>=0)
		{
			FD_CLR(coppiafd[i].fd_latofixed,&all);
			close(coppiafd[i].fd_latofixed);
			coppiafd[i].fd_latofixed=-1;
		}
	}
}

int ricevo_inserisco(int i, uint32_t *pidmsg, uint32_t fd_latoricevere, uint32_t fd_latospedire,
					 uint16_t port_number_latospedire, uint16_t port_number_dest )
{
	/* leggo, calcolo ritardo e metto in lista da spedire */
	char buf[65536];
	struct sockaddr_in From;
	unsigned int Fromlen; int ris;

	/* wait for datagram */
	do {
		memset(&From,0,sizeof(From));
		Fromlen=sizeof(struct sockaddr_in);
		ris = recvfrom ( fd_latoricevere, buf, (int)65536, 0, (struct sockaddr*)&From, &Fromlen);
	} while( (ris<0) && (errno==EINTR) );
	if (ris<0) {
		if(errno==ECONNRESET) {
			perror("recvfrom() failed (ECONNRESET): ");
			fprintf(stderr,"ma non chiudo il socket\n");
			fflush(stderr);
		}
		else {
			perror("recvfrom() failed: "); fflush(stderr);
			close_coppia(i);
		}
		return(0);
	}
	else if(ris>0) {
		int casuale;
		struct timeval now;

		if(ris>=sizeof(*pidmsg))
			memcpy( (char*)pidmsg,buf, sizeof(*pidmsg) );
		else {
			memset((char*)pidmsg,0,sizeof(*pidmsg));
			memcpy( (char*)pidmsg,buf, ris );
		}
		fprintf(stderr,"ricevuto  pkt id %u da port %d\n", *pidmsg, get_local_port(fd_latoricevere));

		/* decido se spedire o scartare */
		gettimeofday(&now,NULL);
		casuale=random()%100 - 3*abs ( sin( (now.tv_sec-coppiafd[i].sec_istcreazione)/8.0 ) );
		/* printf("CASUALE %d\n", casuale); */
		if(casuale<=PERCENTUALE_ERRORE) { /* scarto il 10% dei pkt */
			return(1);
		}
		else {
			/* alloco spazio per il pacchetto dati */
			ELEMENTOLISTA *p;
			struct timeval delay;

			p=(ELEMENTOLISTA *)malloc(sizeof(ELEMENTOLISTA));
			if(p==NULL) { perror("malloc failed: "); exit(9); }
			p->buf=(char*)malloc(ris);
			if(p->buf==NULL) { perror("malloc failed: "); exit(9); }

			p->cmd=CMD_SEND;
			p->len=ris; /* dimensione del pacchetto */
			p->fd=fd_latospedire;
			p->port_number_local=port_number_latospedire;
			p->port_number_dest=port_number_dest;
			/* copio il messaggio ricevuto */
			memcpy(p->buf,buf,ris);
			/* calcolo il ritardo da aggiungere e calcolo il timeout */
			delay.tv_sec=0;
			/* tra 50 e 140 ms */
			delay.tv_usec=	30000 /* BASE */
							+ (random()%20000) /* VARIAZIONE CASUALE */
							+ abs(50000*sin( (now.tv_sec-coppiafd[i].sec_istcreazione)/8.0 )); /* ANDAMENTO */
			/* printf("DELAY %d\n", delay.tv_usec); */
;
			gettimeofday(&(p->timeout),NULL);
			somma(p->timeout,delay,&(p->timeout));
			/* metto in lista */
			aggiungi_in_ordine(p);
			p=NULL;
			return(2);
		}
	}
	return(0);
}

void schedula_creazione_nuova_porta(void)
{
	/* alloco spazio per il pacchetto dati */
	ELEMENTOLISTA *p;
	struct timeval delay;

	p=(ELEMENTOLISTA *)malloc(sizeof(ELEMENTOLISTA));
	if(p==NULL) { perror("malloc failed: "); exit(9); }
	p->buf=NULL;
	p->cmd=CMD_ADDPORT;
	/* calcolo il ritardo da aggiungere e calcolo il timeout */
	delay.tv_sec=0;
	delay.tv_usec=500000; /* mezzo secondo */
	gettimeofday(&(p->timeout),NULL);
	somma(p->timeout,delay,&(p->timeout));
	/* metto in lista */
	aggiungi_in_ordine(p);
	p=NULL;
}


void creazione_nuova_coppia_porte(void)
{
	int i, ris;

	for(i=0;i<MAXNUMCONNECTIONS;i++)
	{
		if(coppiafd[i].attivo==0) /* vuoto */
		{
			struct timeval now;

			ris=UDP_setup_socket_bound( &(coppiafd[i].fd_latomobile), 
										first_local_port_number_mobile_side+counter_localport_mobileside, 65535, 65535 );
			if (!ris)
			{	printf ("UDP_setup_socket_bound() failed\n");
				exit(1);
			}
			coppiafd[i].port_number_latomobile=first_local_port_number_mobile_side+counter_localport_mobileside;
			counter_localport_mobileside++;
			ris=UDP_setup_socket_bound( &(coppiafd[i].fd_latofixed), first_local_port_number_fixed_side+counter_localport_fixedside, 65535, 65535 );
			if (!ris)
			{	printf ("UDP_setup_socket_bound() failed\n");
				exit(1);
			}
			coppiafd[i].port_number_latofixed=first_local_port_number_fixed_side+counter_localport_fixedside;
			counter_localport_fixedside++;
			coppiafd[i].attivo=1;
			FD_SET(coppiafd[i].fd_latomobile,&all);
			if(coppiafd[i].fd_latomobile>maxfd)
				maxfd=coppiafd[i].fd_latomobile;
			FD_SET(coppiafd[i].fd_latofixed,&all);
			if(coppiafd[i].fd_latofixed>maxfd)
				maxfd=coppiafd[i].fd_latofixed;
			gettimeofday(&now,NULL);
			coppiafd[i].sec_istcreazione=now.tv_sec;
			break;
		}
	}
}

void stampa_coppie_porte(void)
{
	int i;

	for(i=0;i<MAXNUMCONNECTIONS;i++)
	{
		if(coppiafd[i].attivo==1)
		{
			fprintf(stderr,"coppia %d: latomobile fd %d port %d - latofixed fd %d port %d \n",
					i, coppiafd[i].fd_latomobile, get_local_port(coppiafd[i].fd_latomobile), 
					coppiafd[i].fd_latofixed, get_local_port(coppiafd[i].fd_latofixed) );
		}
	}
}

int scaduto_timeout(struct timeval *ptimeout)
{
	struct timeval now;
	gettimeofday(&now,NULL);
	if( minoreouguale(ptimeout,&now) )
		return(1);
	else
	{
#ifdef VICDEBUG
		fprintf(stderr,"scaduto_timeout: ptime %d s %d us    now %d s %d us\n",
			(*ptimeout).tv_sec,(*ptimeout).tv_usec,now.tv_sec,now.tv_usec	);
#endif
		return(0);
	}
}

struct timeval compute_timeout_first_pkt(void)
{
	struct timeval now, attesa;

	gettimeofday(&now,NULL);
	attesa=differenza(root->timeout,now);
	return(attesa);
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

#ifdef VICDEBUG
	fprintf(stderr,"send_udp sending to %d\n", port_number_dest);
#endif

	addr_size = sizeof(struct sockaddr_in);
	/* send to the address */
	ris = sendto(socketfd, buf, len , MSG_NOSIGNAL, (struct sockaddr*)&To, addr_size);
	if (ris < 0) {
		printf ("sendto() failed, Error: %d \"%s\"\n", errno,strerror(errno));
		return(0);
	}
	return(1);
}

void notify(int monitorfd, int cmdack, uint32_t idmsg)
{
	ACK ack; int ris;
	
	if(cmdack==1)
		ack.tipoack='A';
	else
		ack.tipoack='N';
	memcpy( (char*)&(ack.id), (char*)&idmsg, sizeof(uint32_t) );
	ris=Sendn(monitorfd,(char*)&ack,sizeof(ACK));
	if(ris!=sizeof(ACK)) { 
		fprintf(stderr,"notify failed: "); 
		exit(99); 
	}
}

#define PARAMETRIDEFAULT "./Monitor.exe 7001 8000 8001 9001 10001"
void usage(void) 
{  printf ("usage: ./Monitor.exe REMOTEPORTMOBILE LOCALPORTMONITOR FIRSTLOCALPORTMOBILESIDE FIRSTLOCALPORTFIXEDSIDE REMOTEPORTFIXED"
		   " PACKETLOSSPERC\n"
				"esempio: "  PARAMETRIDEFAULT " %d\n", PERC_ERR);
}

int main(int argc, char *argv[])
{
	uint16_t	local_port_number_monitor,
				remote_port_number_mobile, remote_port_number_fixed;
	struct sockaddr_in Cli;
	unsigned int len;
	int i, ris;

	if(argc==1) { 
		printf ("uso i parametri di default \n%s %d\n", PARAMETRIDEFAULT, PERC_ERR);
		remote_port_number_mobile = 7001;
		local_port_number_monitor = 8000;
		first_local_port_number_mobile_side = 8001;
		first_local_port_number_fixed_side = 9001;
		remote_port_number_fixed = 10001;
		PERCENTUALE_ERRORE=PERC_ERR;
	}
	else if(argc!=7) { printf ("necessari 6 parametri\n"); usage(); exit(1);  }
	else { /* leggo parametri da linea di comando */
		remote_port_number_mobile = atoi(argv[1]);
		local_port_number_monitor = atoi(argv[2]);
		first_local_port_number_mobile_side = atoi(argv[3]);
		first_local_port_number_fixed_side = atoi(argv[4]);
		remote_port_number_fixed = atoi(argv[5]);
		PERCENTUALE_ERRORE=atoi(argv[6]);
	}
	for(i=0;i<MAXNUMCONNECTIONS;i++) {
		coppiafd[i].fd_latomobile=-1;
		coppiafd[i].fd_latofixed=-1;
		coppiafd[i].attivo=0;
	}
	root=NULL;
	init_random();
	if ((signal (SIGHUP, sig_print)) == SIG_ERR) { perror("signal (SIGHUP) failed: "); exit(2); }
	if ((signal (SIGINT, sig_print)) == SIG_ERR) { perror("signal (SIGINT) failed: "); exit(2); }
	if ((signal (SIGTERM, sig_print)) == SIG_ERR) { perror("signal (SIGTERM) failed: "); exit(2); }
	ris=TCP_setup_socket_listening( &listening_monitorfd, local_port_number_monitor, 300000, 300000, 1);
	if (!ris)
	{	printf ("TCP_setup_socket_listening() failed\n");
		exit(1);
	}
#ifdef VICDEBUG
	fprintf(stderr,"socket listening %d\n", listening_monitorfd);
	fflush(stderr);
#endif

	/* attendo la connessione dal lato mobile */
	do { memset (&Cli, 0, sizeof (Cli));		
		len = sizeof (Cli);
		monitorfd = accept ( listening_monitorfd, (struct sockaddr *) &Cli, &len);
	} while ( (monitorfd<0) && (errno==EINTR) );
	if (monitorfd < 0 ) {	
		perror("accept() failed: \n");
		exit (1);
	}
	/* chiusura del socket TCP listening e setup dei socket UDP	*/
	close(listening_monitorfd);
	FD_CLR(listening_monitorfd,&all);
	listening_monitorfd=-1;
#ifdef VICDEBUG
	fprintf(stderr,"socket monitorfd %d\n", monitorfd);
	fflush(stderr);
#endif

	FD_ZERO(&all);
	/* stdin */
	FD_SET(0,&all);
	maxfd=0;
	FD_SET(monitorfd,&all);
	maxfd=monitorfd;
	for(i=0;i<=1;i++) creazione_nuova_coppia_porte();
	/* differenzio le due coppie */
	coppiafd[0].sec_istcreazione-=10;

#ifdef VICDEBUG
	stampa_coppie_porte();
#endif

	ris=send_configurazione(monitorfd);
#ifdef VICDEBUG
	fprintf(stderr,"send_configurazione sent %d\n", ris);
#endif

	for(;;)
	{
ripeti: ;
		do {
			rdset=all;
#ifdef VICDEBUG
			stampa_fd_set("rdset prima",&rdset);
#endif
			if(root!=NULL) {
				struct timeval timeout;
				timeout=compute_timeout_first_pkt();
#ifdef VICDEBUG
				fprintf(stderr,"set timeout %d sec %d usec\n", timeout.tv_sec,timeout.tv_usec );
#endif
				ris=select(maxfd+1,&rdset,NULL,NULL,&timeout);
			}
			else {
#ifdef VICDEBUG
				fprintf(stderr,"set timeout infinito\n");
#endif
				ris=select(maxfd+1,&rdset,NULL,NULL,NULL);
			}
		} while( (ris<0) && (errno==EINTR) && (printed==0) );
		if(ris<0) {
			perror("select failed: ");
			exit(1);
		}
		if(printed!=0) {
			fprintf(stderr,"esco da select dopo avere gia' segnalato la chiusura, TERMINO\n");
			exit(1);
		}
#ifdef VICDEBUG
		stampa_fd_set("rdset dopo",&rdset);
#endif

		/* se arriva qualcosa dalla connessione TCP con lato mobile, termino!!!! */
		if( FD_ISSET(monitorfd,&rdset) )
		{
			char buf[1000]; int ris;
			fprintf(stderr,"in arrivo qualcosa dalla connessione TCP del mobile: che e'?\n");
			fflush(stderr);
			/* ris=recv(monitorfd,buf,1000,MSG_DONTWAIT); */
			ris=read(monitorfd,buf,1000);
			if(ris<0) {
				if(errno==EINTR) { fprintf(stderr,"recv monitorfd EINTR, continuo\n"); break; }
				else if(errno==ECONNRESET) { perror("recv monitorfd failed: ECONNRESET "); fprintf(stderr,"NON TERMINO\n"); }
				else  {	int myerrno=errno; perror("recv monitorfd failed: "); fprintf(stderr,"errno %d\n",myerrno); 
						fprintf(stderr,"NON TERMINO\n"); exit(11); 
				}
			}
			else if(ris==0) { fprintf(stderr,"recv ZERO from monitorfd: TERMINO\n"); exit(12); }
			else {
				int j;
				fprintf(stderr,"recv %d from monitorfd: ", ris);
				for(j=0;j<ris;j++) fprintf(stderr,"%c",buf[j]);
				fprintf(stderr,"\n");
			}
			break; /* esco dal for piu' esterno */
		}
		/* se arriva qualcosa dallo stdin, chiudo una coppia di porte 
		   e schedulo la creazione di una nuova porta
		*/

		if( FD_ISSET(0,&rdset) )
		{
			int fatto=0;
			char *ptr, str[512];

			ptr=fgets(str,512,stdin);
			if(ptr!=NULL) {
				fprintf(stderr,"leggo da stdin\n");
				fflush(stderr);
				for(i=0;i<MAXNUMCONNECTIONS;i++)	
				{
					if(coppiafd[i].attivo==1)
					{
						close_coppia(i);
						send_configurazione(monitorfd);
						fprintf(stderr,"chiusa coppia di porte\n");
						schedula_creazione_nuova_porta();
						fatto=1;
						break;
					}
				}
				if(fatto==0) fprintf(stderr,"nessuna coppia di porte attive\n");
				else goto ripeti; /* ho chiuso coppia, lo fd_set rdset ora e' sballato, quindi rifaccio loop */
			}
		}

		/* spedisco o elimino i pkt da spedire */
		while( (root!=NULL) && scaduto_timeout( &(root->timeout) )   ) {
			if(root->cmd==CMD_SEND) {
				if( check_port(root->port_number_local) ) {
					ris=send_udp(root->fd,root->buf,root->len,root->port_number_local,"127.0.0.1",root->port_number_dest);
					fprintf(stderr,"pkt id %u sent %d from %d to %d\n", *((uint32_t*)(root->buf)), ris, root->port_number_local, root->port_number_dest );
				}
				free_pkt(&root);
			}
			else if(root->cmd==CMD_ADDPORT) {
				/* inserisco una nuova coppia di porte */
				creazione_nuova_coppia_porte();
				send_configurazione(monitorfd);
				free_pkt(&root);
			}
		}

		/* gestione pacchetti in arrivo */
		for(i=0;i<MAXNUMCONNECTIONS;i++)	
		{
			uint32_t idmsg;
			if(coppiafd[i].attivo==1)
			{
				if( FD_ISSET(coppiafd[i].fd_latomobile,&rdset) )
				{
					#ifdef VICDEBUG
					fprintf(stderr,"leggo da lato mobile\n");
					#endif
					/* leggo, calcolo ritardo e metto in lista da spedire verso il fixed */
					ris=ricevo_inserisco(	i, &idmsg, coppiafd[i].fd_latomobile, coppiafd[i].fd_latofixed,
											coppiafd[i].port_number_latofixed, remote_port_number_fixed );
					if(ris==0)		{ P("NULLAmobile");  ; }	/* non ricevuto niente, o errore sul socket */
					else if(ris==1)	{ 
						fprintf(stderr,"SCARTO idmsg %u da latomobile\n", idmsg);
						notify(monitorfd,0,idmsg); /* notifica scartato */
						numscartati++;
					}
					else if(ris==2)	{ 
						#ifdef VICDEBUG
						P("SPEDISCOmobile"); 
						#endif
						notify(monitorfd,1,idmsg); /* notifica spedito */
						numspediti++;
					}
					else { P("CHEE'mobile "); ;}
				}
			}

			if(coppiafd[i].attivo==1)
			{
				if( FD_ISSET(coppiafd[i].fd_latofixed,&rdset) )
				{
					#ifdef VICDEBUG
					fprintf(stderr,"leggo da lato fixed\n");
					#endif
					/* leggo, calcolo ritardo e metto in lista da spedire */
					ris=ricevo_inserisco(	i, &idmsg, coppiafd[i].fd_latofixed, coppiafd[i].fd_latomobile, 
											coppiafd[i].port_number_latomobile, remote_port_number_mobile );
					if(ris==0)		{ P("NULLAfixed");  ; }	/* non ricevuto niente, o errore sul socket */
					else if(ris==1)	{ 
						fprintf(stderr,"SCARTO idmsg %u da latofixed\n", idmsg);
						/* notify(monitorfd,0,idmsg); NO notifica scartato */
						;
					}
					else if(ris==2)	{ 
						#ifdef VICDEBUG
						P("SPEDISCOmobile"); 
						#endif
						/* notify(monitorfd,1,idmsg); NO notifica spedito */
						;
					}
					else { P("CHEE'fixed "); ;}
				}

			}
		} /* fine for i */

	} /* fine for ;; */

	for(i=0;i<MAXNUMCONNECTIONS;i++)
		if(coppiafd[i].attivo==1)
			close_coppia(i);
	return(0);
}

