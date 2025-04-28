
PROG=PrintF

PCLIB=pClib
PCLIB_TARGETS=${PCLIB} ${PCLIB}.a ${PCLIB}.so 

SRC=${PROG}.c

CFLAGS=-Wall 

all: ${PROG} ${PCLIB_TARGETS}

${PROG}: ${SRC}
	cc -DTEST_APP ${CFLAGS} -o $@ $<

${PCLIB}: ${PCLIB}.c
	gcc ${CFLAGS} -DTEST_APP $< -o $@ -nostdlib -fPIE

${PCLIB}.o: ${PCLIB}.c
	gcc ${CFLAGS} -c $<

${PCLIB}.a: ${PCLIB}.o
	ar rcs $@ $^

${PCLIB}.so: ${PCLIB}.o
	gcc -nostdlib -fPIE -shared -o $@ $^

clean:
	rm -f ${PROG} ${PCLIB_TARGETS} *.o
