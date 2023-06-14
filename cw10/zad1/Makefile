CC = gcc
CFLAGS = -Wall
PORT = 9000

all: server client

server: server.c
	$(CC) $(CFLAGS) server.c -o server

client: client.c
	$(CC) $(CFLAGS) client.c -o client

test: all
	./server $(PORT)

test_client: all
	./client name 127.0.0.1 $(PORT)

clean:
	rm *.o server client logs.txt