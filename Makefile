INCLUDES=-Iinclude
LDFLAGS=-lpthread -lm
CFLAGS=${INCLUDES} -pipe -Wall -Wunused -pedantic -ggdb -DDEBUG
SUBDIRS=test
EXECUTABLES=MobileLoadBalancer FixedLoadBalancer
UNITTESTS=initconn
.PHONY: clean cleanindent $(SUBDIRS)

all: subdirs $(EXECUTABLES)

utests: $(UNITTESTS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

%.c.o:

MobileLoadBalancer: src/Common.o src/MobileLoadBalancer.o
	${CC} ${LDFLAGS} $^ -o $@
FixedLoadBalancer: src/Common.o src/FixedLoadBalancer.o
	${CC} ${LDFLAGS} $^ -o $@

initconn: src/Common.o utests/initconn.o
	${CC} ${LDFLAGS} $^ -o $@

#GNU indent only
indent:
	indent src/*.c include/*.h utests/*.c -i8 -bli0 -br -npsl -npcs
cleanindent:
	-rm -f src/*.c~ include/*.h~ utests/*.c~
clean:	cleanindent
	-rm -f core* *.stackdump delaymobile.txt
	-rm -f $(EXECUTABLES) src/*.o
	-rm -f $(UNITTESTS) utests/*.o
	$(MAKE) -C $(SUBDIRS) clean
