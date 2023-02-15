.PHONY: client server

CFLAGS=-Wall -Wextra -Wpedantic -DDEBUG -g

all: client server
client: 
	gcc $(CFLAGS) -o client client.c
	
