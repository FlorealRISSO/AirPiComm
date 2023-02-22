.PHONY: client server

CFLAGS= -std=gnu99 -Wall -Wextra -Wpedantic -DDEBUG -g

all: client server
client:
	gcc $(CFLAGS) -o client client.c

server:
	gcc $(CFLAGS) -o server server.c

format:
	astyle --style=kr -xf -s4 -k3 -n -Z -Q *.[ch]

