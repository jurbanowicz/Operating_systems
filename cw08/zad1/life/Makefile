CC = gcc
CFLAGS = -Wall

all: life
	
life: grid.c life.c
	$(CC) $(CFLAGS) grid.c life.c -o life -lncurses

test: all
	./life

clean:
	rm -f life
