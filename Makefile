INCLUDES=-Iinclude
LDFLAGS=-lpthread -lm
CFLAGS=${INCLUDES} -Wall -Wunused -pedantic -ggdb
SUBDIRS=test
EXECUTABLES=MobileLoadBalancer FixedLoadBalancer
.PHONY: clean cleanindent $(SUBDIRS)

all: subdirs $(EXECUTABLES)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

%.c.o:

MobileLoadBalancer: src/Common.o src/MobileLoadBalancer.o
	${LD} ${LDFLAGS} $^ -o $@
FixedLoadBalancer: src/Common.o src/FixedLoadBalancer.o
	${LD} ${LDFLAGS} $^ -o $@

#GNU indent only
indent:
	indent src/*.c include/*.h -i8 -bli0 -br -npsl -npcs
cleanindent:
	-rm -f src/*.c~ include/*.h~
clean:	cleanindent
	-rm -f core* *.stackdump delaymobile.txt
	-rm -f MobileLoadBalancer FixedLoadBalancer src/*.o
	$(MAKE) -C $(SUBDIRS) clean
