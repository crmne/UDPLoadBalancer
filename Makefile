CC=@echo "\033[32m  CC	$@\033[00m";cc
LD=@echo "\033[33m  LD	$@\033[00m";cc
PROTOCOL=dumb
INCLUDES=-Iinclude -Iinclude/protocols
LDFLAGS=-lpthread -lm
DEFINES=-DPROTO_H=\"${PROTOCOL}.h\" -DDEBUG
CFLAGS=${INCLUDES} -O3 -pipe -Wall -ansi -pedantic -ggdb ${DEFINES}
SUBDIRS=disttest
CORE=src/conn.o src/comm.o src/queue.o src/timeval.o src/protocols/$(PROTOCOL).o
SOURCES=src/*.c src/protocols/*.c include/*.h include/protocols/*.h utests/*.c
EXECUTABLES=mlb flb
UNITTESTS=initconn select queues memcpy lb_mobile lb_fixed
.PHONY: clean cleanindent $(SUBDIRS)

all: subdirs $(EXECUTABLES)

utests: $(UNITTESTS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

%.c.o:

mlb: $(CORE) src/mlb.o
	${LD} ${LDFLAGS} $^ -o $@

flb: $(CORE) src/flb.o
	${LD} ${LDFLAGS} $^ -o $@

initconn: $(CORE) utests/initconn.o
	${LD} ${LDFLAGS} $^ -o $@

select: $(CORE) utests/select.o
	${LD} ${LDFLAGS} $^ -o $@

queues: $(CORE) utests/queues.o
	${LD} ${LDFLAGS} $^ -o $@

memcpy: $(CORE) utests/memcpy.o
	${LD} ${LDFLAGS} $^ -o $@

lb_mobile: $(CORE) utests/lb_mobile.o
	${LD} ${LDFLAGS} $^ -o $@
lb_fixed: $(CORE) utests/lb_fixed.o
	${LD} ${LDFLAGS} $^ -o $@


indent:
	@if which indent && [ "`indent --version | cut -d' ' -f1`" == "GNU" ]; \
	then indent ${SOURCES} -kr -nhnl -bad -bap; \
	else for i in ${SOURCES}; do \
	indent -nbad -bap -nbc -br -brs -c33 -cd33 -ncdb -ce -ci4 -cli0 -d0 -di1 -nfc1 -i4 -ip0 -l75 -lp -npcs -npsl -nsc -nsob $$i; done; fi

cleanindent:
	-for i in ${SOURCES}; do rm -f $$i~; done
	-rm -f *.BAK

clean:	cleanindent
	-rm -f core* *.stackdump delaymobile.txt delayfixed.txt
	-rm -f $(EXECUTABLES) src/*.o src/protocols/*.o
	-rm -f $(UNITTESTS) utests/*.o

cleansubdirs:
	$(MAKE) -C $(SUBDIRS) clean
