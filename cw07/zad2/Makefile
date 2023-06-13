CC = gcc
CFLAGS = -Wall

all:
	$(CC) $(CFLAGS) -c -lrt main.c
	$(CC) $(CFLAGS) main.o -o main

test: all
	./main 3 2 2