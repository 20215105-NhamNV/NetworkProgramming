CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm 

all: server client1 client2

server: server.c
	${CC} -o server server.c

client1: client1.c
	${CC} -o client1 client1.c

client2: client2.c
	${CC} -o client2 client2.c

clean:
	rm -f *.o *~