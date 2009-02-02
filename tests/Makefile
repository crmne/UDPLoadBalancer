PROTOCOL=dumb
INCLUDES=-I../include -I../include/protocols
LDFLAGS=-lpthread -lm
DEFINES=-DPROTO_H=\"${PROTOCOL}.h\" -DDEBUG
CFLAGS=${INCLUDES} -O3 -pipe -Wall -ansi -pedantic -ggdb ${DEFINES}
CORE=../src/conn.o ../src/comm.o ../src/queue.o ../src/timeval.o ../src/protocols/$(PROTOCOL).o
CC=@echo "Compiling $@ ...";cc
LD=@echo "Linking $@ ...";cc
TESTS=initconn select queues memcpy lb_mobile lb_fixed packloss

packloss: packloss.o
	${LD} ${LDFLAGS} $^ -o $@

all: $(TESTS)

initconn: $(CORE) initconn.o
	${LD} ${LDFLAGS} $^ -o $@

select: $(CORE) select.o
	${LD} ${LDFLAGS} $^ -o $@

queues: $(CORE) queues.o
	${LD} ${LDFLAGS} $^ -o $@

memcpy: $(CORE) memcpy.o
	${LD} ${LDFLAGS} $^ -o $@

lb_mobile: $(CORE) lb_mobile.o
	${LD} ${LDFLAGS} $^ -o $@

lb_fixed: $(CORE) lb_fixed.o
	${LD} ${LDFLAGS} $^ -o $@

clean:
	-rm *.o $(TESTS)
