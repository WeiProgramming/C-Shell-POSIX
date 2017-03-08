# The official makefile for Program 2, CS570

PROGRAM = p2
CC = gcc
CFLAGS = -g

${PROGRAM}:	${PROGRAM}.o getword.o
		${CC} ${CFLAGS} ${PROGRAM}.o getword.o -o ${PROGRAM} -lm

getword.o:	getword.h
${PROGRAM}.o:	${PROGRAM}.h getword.h

clean:
		rm -f *.o ${PROGRAM} your.output*

lint:
		lint getword.c ${PROGRAM}.c
