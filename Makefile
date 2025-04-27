
PROG=PrintF

SRC=${PROG}.c

CFLAGS=-Wall -O2 -DTEST_APP

all: ${PROG}

${PROG}: ${SRC}
	cc ${CFLAGS} -o $@ $<

pClib: pClib.c
	gcc -DTEST_APP -Wall $< -o $@ -nostdlib -fPIE
