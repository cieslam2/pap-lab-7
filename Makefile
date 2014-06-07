SHELL = /bin/bash
CC = gcc
CFLAGS = -Wall --pedantic -I/local/Cellar/go/1.2.2/libexec/pkg/darwin_amd64/ -lcrypto -lssl
TARGET = bin/
SRC = source/

zad1:
	${CC} ${CFLAGS} ${SRC}zad1/bio_client.c -o ${TARGET}zad1/bio_client.o
	${CC} ${CFLAGS} ${SRC}zad1/bio_server.c -o ${TARGET}zad1/bio_server.o
	${CC} ${CFLAGS} ${SRC}zad1/fd_client.c -o ${TARGET}zad1/fd_client.o
	${CC} ${CFLAGS} ${SRC}zad1/fd_server.c -o ${TARGET}zad1/fd_server.o

clean:
	find ${TARGET} -name "*.o" | xargs rm -rf

