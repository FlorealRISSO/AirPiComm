.PHONY: client server

CFLAGS= -std=gnu11 -Wall -Wextra -Wpedantic -DDEBUG -g

all: client server
client:
	gcc $(CFLAGS) -o client client.c

server:
	gcc $(CFLAGS) -o server server.c
	
