INCLUDES=-Iinclude
LDFLAGS=-lpthread -lm
CFLAGS=${INCLUDES} -pipe -Wall -Wunused -pedantic -ggdb -DDEBUG
SUBDIRS=test
PROTOCOL=DumbProto
CORE=src/Common.o src/Queues.o src/$(PROTOCOL).o
EXECUTABLES=MobileLoadBalancer FixedLoadBalancer
UNITTESTS=initconn select queues memcpy
.PHONY: clean cleanindent $(SUBDIRS)

all: subdirs $(EXECUTABLES)

utests: $(UNITTESTS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

%.c.o:

MobileLoadBalancer: $(CORE) src/MobileLoadBalancer.o
	${CC} ${LDFLAGS} $^ -o $@
FixedLoadBalancer: $(CORE) src/FixedLoadBalancer.o
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
	indent src/*.c include/*.h utests/*.c -i8 -bli0 -br -npsl -npcs
cleanindent:
	-rm -f src/*.c~ include/*.h~ utests/*.c~
clean:	cleanindent
	-rm -f core* *.stackdump delaymobile.txt delayfixed.txt
	-rm -f $(EXECUTABLES) src/*.o
	-rm -f $(UNITTESTS) utests/*.o
	$(MAKE) -C $(SUBDIRS) clean
