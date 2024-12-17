CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm 

all: server client

server: server.c
	${CC} -o server server.c

client: client.c
	${CC} -o client client.c

clean:
	rm -f *.o *~