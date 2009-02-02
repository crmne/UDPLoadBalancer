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
	@echo Done. Hint: use tests/test.sh then tests/plot.sh
subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

%.c.o:

mlb: $(CORE) src/mlb.o
	${LD} ${LDFLAGS} $^ -o $@

flb: $(CORE) src/flb.o
	${LD} ${LDFLAGS} $^ -o $@

indent:
	@if which indent && [ "`indent --version | cut -d' ' -f1`" == "GNU" ]; \
	then indent ${SOURCES} -kr -nhnl -bad -bap; \
	else for i in ${SOURCES}; do \
	indent -nbad -bap -nbc -br -brs -c33 -cd33 -ncdb -ce -ci4 -cli0 -d0 -di1 -nfc1 -i4 -ip0 -l75 -lp -npcs -npsl -nsc -nsob $$i; done; fi

cleanplot:
	-rm *.png fit.log

cleanindent:
	-for i in ${SOURCES}; do rm -f $$i~; done
	-rm -f *.BAK

cleansubdirs:
	-for i in $(SUBDIRS); do $(MAKE) -C "$${i}" clean; done

clean:	cleansubdirs cleanindent cleanplot
	-rm -f core* *.stackdump *.txt
	-rm -f $(EXECUTABLES) src/*.o src/protocols/*.o
