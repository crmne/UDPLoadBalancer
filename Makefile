PROTOCOL=dumb
INCLUDES=-Iinclude -Iinclude/protocols
LDFLAGS=-lpthread -lm
DEFINES=-DPROTO_H=\"${PROTOCOL}.h\" -DDEBUG
CFLAGS=${INCLUDES} -O3 -pipe -Wall -ansi -pedantic -ggdb ${DEFINES}
SUBDIRS=disttest tests
CORE=src/conn.o src/comm.o src/queue.o src/timeval.o src/protocols/$(PROTOCOL).o
SOURCES=src/*.c src/protocols/*.c include/*.h include/protocols/*.h
EXECUTABLES=mlb flb
CC=@echo "Compiling $@ ...";cc
LD=@echo "Linking $@ ...";cc
.PHONY: clean cleanindent cleanplot cleansubdirs $(SUBDIRS)

all: subdirs $(EXECUTABLES)
	@echo Done.
subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

%.c.o:

mlb: $(CORE) src/mlb.o
	${LD} ${LDFLAGS} $^ -o $@

flb: $(CORE) src/flb.o
	${LD} ${LDFLAGS} $^ -o $@

indent:
	@if which indent && \
		[ "`indent --version 2> /dev/null | cut -d' ' -f1`" == "GNU" ];\
	then indent ${SOURCES} -kr -nhnl -nut; \
	else echo "Sorry, GNU indent required!"; fi

cleanplot:
	-rm *.png fit.log 2> /dev/null

cleanindent:
	-for i in ${SOURCES}; do rm -f $$i~ 2> /dev/null; done

cleansubdirs:
	-for i in $(SUBDIRS); do $(MAKE) -C "$${i}" clean; done

clean:	cleansubdirs cleanindent cleanplot
	-rm -f core* *.stackdump *.txt 2> /dev/null
	-rm -f $(EXECUTABLES) src/*.o src/protocols/*.o 2> /dev/null
