#define ERR_SELECT 2, "select()"
#define ERR_MONPACK 3, "Monitor packet not understood"
#define ERR_NOFDSET 4, "Incoming data, but in no controlled fd. Strange"
#define ERR_TRANSPORT 5, "Invalid transport"
#define ERR_READZERO 6, "Nothing received, maybe the other end is down? Exiting."
#define ERR_RECV 7, "failed reading %d bytes from socket %d", n, socketfd
#define ERR_SEND 8, "failed writing %d bytes from socket %d", size, socketfd
#define ERR_GETTIMEOFDAY 10, "gettimeofday()"
#define ERR_INETADDR 11, "inet_addr()"
#define ERR_SOCKET 12, "socket() error (type %d)", type
#define ERR_SETSOCKOPT 13, "setsockopt() error (type %d, socketfd %d)", type, socketfd
#define ERR_BIND 14, "bind() error (port %d, socketfd %d)", port, socketfd
#define ERR_LISTEN 15, "listen() error (port %d, socketfd %d)", port, socketfd
#define ERR_CONNECT 16, "connect() error (port %d, socketfd %d)", port, socketfd
#define ERR_ACCEPT 17, "accept() error (socketfd %d)", socketfd
#define ERR_MALLOC 18, "malloc()"

/* WARNINGS */
#define WARN_2PKTQ "Ignoring insertion of already in queue packet (id %u)", packet->id
