#Makefile
CFLAGS=-Wall

all:		polipc

debug:		polipcd

polipcd:	polipc.o 
	$(CC)	-g polipc.c -o polipc 

polipc:		polipc.o
	$(CC)  -o polipc polipc.o -lnewt 


polipc.o:	polipc.c 

install:
	 cp -f polipc /usr/bin/

clean:
	rm -f polipc core *.o
	
