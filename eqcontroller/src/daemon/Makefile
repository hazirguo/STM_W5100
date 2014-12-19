#
# Makefile for the controller daemon.
#
CFLAGS=-g -Wall
LIBS= -lnsl -lwrap
CC = gcc
PREFIX = /usr/local/eqcontroller

all: labeqd eqslave

labeqd: logutils.o labeqd.o
	${CC} -o labeqd ${LIBS} logutils.o labeqd.o

eqslave: logutils.o eqslave.o
	${CC} -o eqslave ${LIBS} logutils.o eqslave.o

clean:
	rm -f *.o *~ core labeqd eqslave

labeqd.o: labeqd.c ../include/labeqc.h
	${CC} -c ${CFLAGS} labeqd.c

eqslave.o: eqslave.c ../include/labeqc.h
	${CC} -c ${CFLAGS} eqslave.c

logutils.o: logutils.c ../include/labeqc.h
	${CC} -c ${CFLAGS} logutils.c


install		:	all
	install labeqd eqslave $(PREFIX)/bin/
