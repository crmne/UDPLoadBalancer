INCLUDES=-I../include -I../include/protocols
LDFLAGS=-lpthread -lm
DEFINES= -DDEBUG
CFLAGS=${INCLUDES} -O2 -pipe -Wall -Werror -ansi -pedantic -ggdb ${DEFINES}
CORE=../src/conn.o ../src/comm.o ../src/queue.o ../src/timeval.o
CC=@echo "Compiling $@ ...";cc
LD=@echo "Linking $@ ...";cc
TESTS=initconn select queues memcpy packloss

all: $(TESTS)
packloss: packloss.o
	${LD} ${LDFLAGS} $^ -o $@

initconn: $(CORE) initconn.o
	${LD} ${LDFLAGS} $^ -o $@

select: $(CORE) select.o
	${LD} ${LDFLAGS} $^ -o $@

queues: $(CORE) queues.o
	${LD} ${LDFLAGS} $^ -o $@

memcpy: $(CORE) memcpy.o
	${LD} ${LDFLAGS} $^ -o $@

clean:
	-rm *.o $(TESTS) 2> /dev/null
