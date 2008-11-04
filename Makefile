INCLUDES=-Iinclude
LDFLAGS=-lpthread -lm
CFLAGS=${INCLUDES} -Wall -Wunused -pedantic -ggdb
SUBDIRS=test
EXECUTABLES=MobileLoadBalancer FixedLoadBalancer
.PHONY: clean $(SUBDIRS)

all: subdirs $(EXECUTABLES)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

%.c.o:

MobileLoadBalancer: src/MobileLoadBalancer.o
	${LD} ${LDFLAGS} $^ -o $@
FixedLoadBalancer: src/FixedLoadBalancer.o
	${LD} ${LDFLAGS} $^ -o $@

indent:
	#sed -i '/^$$/d' src/*.c include/*.h
	astyle -n --style=ansi src/*.c include/*.h

clean:	
	-rm -f core* *.stackdump delaymobile.txt
	-rm -f MobileLoadBalancer FixedLoadBalancer src/*.o
	$(MAKE) -C $(SUBDIRS) clean
