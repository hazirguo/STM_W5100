CC= gcc
CFLAGS= -O -Wall -g
LIBS= -lnsl
PREFIX	= /usr/local/eqcontroller

all: test_labeqc .install/run_eqc

install: all
	install -c test_labeqc ${PREFIX}/bin
	install -c .install/run_eqc ${PREFIX}/bin
	(cd scripts; tar -cf - .) | (cd ${PREFIX}/test-scripts; tar -xpf -)

clean::
	rm -f *.o *~ core test_labeqc
	rm -fr .install

.install/run_eqc:	run_eqc
	@if [ ! -d .install ]; then mkdir -p .install; fi
	@cat run_eqc | sed 's^@PREFIX@^$(PREFIX)^g' > .install/run_eqc

test_labeqc: test_labeqc.o lists.o
	gcc $(CFLAGS) lists.o test_labeqc.o -o test_labeqc -lm

test_labeqc.o: test_labeqc.c ../../include/labeqc.h lists.c lists.h
	${CC} -c $(CFLAGS) test_labeqc.c -o test_labeqc.o

lists.o: lists.c lists.h
	${CC} -c $(CFLAGS) lists.c -o lists.o
