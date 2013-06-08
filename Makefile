SHELL = /bin/bash

CC = gcc
CFLAGS = -Wall --pedantic

TARGET = bin/
SRC = source/

all: generate

generate:
	openssl req –newkey rsa:2048 –keyout root_key.pem –out root_request.pem

clean:
	find ${TARGET} -name "*.o" | xargs rm -rf

.PHONY: all clean
