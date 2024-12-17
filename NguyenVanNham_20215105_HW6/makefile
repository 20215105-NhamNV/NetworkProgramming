# CC = gcc
# CFLAGS = -Wall

# all: server client

# server: server.c
# 	$(CC) $(CFLAGS) -o server server.c -lssl -lcrypto

# client: client.c
# 	$(CC) $(CFLAGS) -o client client.c

# clean:
# 	rm -f server client
CC = gcc
CFLAGS = -Wall -g
TARGETS = server client

FILESRCS = sha256.c
FILEOS = $(FILESRCS:.c=.o)

all: $(TARGETS)

server: server.o $(FILEOS)
	$(CC) $(CFLAGS) -o $@ server.o $(FILEOS)

client: client.o $(FILEOS)
	$(CC) $(CFLAGS) -o $@ client.o $(FILEOS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(TARGETS)
