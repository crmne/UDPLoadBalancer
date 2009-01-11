INCLUDES=-Iinclude
LDFLAGS=-lpthread -lm
CFLAGS=${INCLUDES} -pipe -Wall -Wunused -pedantic -ggdb -DDEBUG
SUBDIRS=disttest
PROTOCOL=dumb
CORE=src/conn.o src/comm.o src/queue.o src/utils.o src/protocols/$(PROTOCOL).o
EXECUTABLES=mlb flb
UNITTESTS=initconn select queues memcpy
.PHONY: clean cleanindent $(SUBDIRS)

all: subdirs $(EXECUTABLES)

utests: $(UNITTESTS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

%.c.o:

mlb: $(CORE) src/mlb.o
	${CC} ${LDFLAGS} $^ -o $@
flb: $(CORE) src/flb.o
	${CC} ${LDFLAGS} $^ -o $@

initconn: $(CORE) utests/initconn.o
	${CC} ${LDFLAGS} $^ -o $@

select: $(CORE) utests/select.o
	${CC} ${LDFLAGS} $^ -o $@

queues: $(CORE) utests/queues.o
	${CC} ${LDFLAGS} $^ -o $@

memcpy: $(CORE) utests/memcpy.o
	${CC} ${LDFLAGS} $^ -o $@




#GNU indent only
indent:
	@if [ "`indent --version 2> /dev/null | cut -d' ' -f1 `" == "GNU" ]; then indent src/*.c include/*.h utests/*.c -i8 -bli0 -br -npsl -npcs; else echo "Sorry, GNU indent required!"; fi
cleanindent:
	-rm -f src/*.c~ src/protocols/*.c~ include/*.h~ utests/*.c~
clean:	cleanindent
	-rm -f core* *.stackdump delaymobile.txt delayfixed.txt
	-rm -f $(EXECUTABLES) src/*.o src/protocols/*.o
	-rm -f $(UNITTESTS) utests/*.o
	$(MAKE) -C $(SUBDIRS) clean
