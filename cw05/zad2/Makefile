CC = gcc
CFLAGS = -Wall

all:
	$(CC) $(CFLAGS) zadanie2.c -o zadanie2

test: all
	./zadanie2 0.0000000001 16

raport: all
	./zadanie2 0.00000001 2 > raport.txt
	./zadanie2 0.00000001 4 >> raport.txt
	./zadanie2 0.00000001 8 >> raport.txt
	./zadanie2 0.00000001 16 >> raport.txt

	./zadanie2 0.0000000001 2 >> raport.txt
	./zadanie2 0.0000000001 4 >> raport.txt
	./zadanie2 0.0000000001 8 >> raport.txt
	./zadanie2 0.0000000001 16 >> raport.txt

clean:
	rm zadanie2
