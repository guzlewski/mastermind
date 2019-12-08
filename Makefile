all: client server

nocolor: server client.c
	gcc -D NOCOLOR -Wall client.c -o client.out

client: client.c shared.h
	gcc -Wall client.c -o client.out

server: server.c shared.h
	gcc -Wall server.c -o server.out

shared.h: