GCCFLAGS= -Wall -Wunused -pedantic -ggdb
LINKERFLAGS=-lpthread -lm

all:  Appmobile.exe Appfixed.exe Monitor.exe

Appmobile.exe: Appmobile.o Util.o
	gcc -o Appmobile.exe ${GCCFLAGS} ${LINKERFLAGS} Appmobile.o Util.o

Appmobile.o: Appmobile.c Util.h
	gcc -c ${GCCFLAGS} Appmobile.c

Appfixed.exe: Appfixed.o Util.o
	gcc -o Appfixed.exe ${GCCFLAGS} ${LINKERFLAGS} Appfixed.o Util.o

Appfixed.o: Appfixed.c Util.h
	gcc -c ${GCCFLAGS} Appfixed.c

Monitor.o:	Monitor.c Util.h
	gcc -c ${GCCFLAGS} Monitor.c

Monitor.exe:	Monitor.o Util.o
	gcc -o Monitor.exe ${GCCFLAGS} ${LINKERFLAGS} Util.o Monitor.o 

Util.o: Util.c 
	gcc -c ${GCCFLAGS}  Util.c

clean:	
	rm -f core* *.stackdump
	rm -f *.exe
	rm -f Appmobile.o Appmobile.exe
	rm -f Appfixed.o Appfixed.exe
	rm -f Monitor.o Monitor.exe
	rm -f Util.o

