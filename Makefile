SHELL = /bin/bash
CC = gcc-4.2
CFLAGS = -Wall --pedantic
TARGET = bin/
SRC = source/

zad1:
	${CC} ${CFLAGS} ${SRC}zad1/bio_client.c -o ${TARGET}zad1/bio_client.o
	${CC} ${CFLAGS} ${SRC}zad1/bio_server.c -o ${TARGET}zad1/bio_server.o
	${CC} ${CFLAGS} ${SRC}zad1/fd_client.c -o ${TARGET}zad1/fd_client.o
	${CC} ${CFLAGS} ${SRC}zad1/fd_server.c -o ${TARGET}zad1/fd_server.o

clean:
	find ${TARGET} -name "*.o" | xargs rm -rf

