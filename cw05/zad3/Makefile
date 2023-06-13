CC = gcc
CFLAGS = -Wall

all:
	$(CC) $(CFLAGS) integrate.c -o integrate
	$(CC) $(CFLAGS) zadanie3.c -o zadanie3

test: all
	./zadanie3 0.00000001 8

raport: all
	./zadanie3 0.00000001 2 > raport.txt
	./zadanie3 0.00000001 4 >> raport.txt
	./zadanie3 0.00000001 8 >> raport.txt
	./zadanie3 0.00000001 16 >> raport.txt

	./zadanie3 0.0000000001 2 >> raport.txt
	./zadanie3 0.0000000001 4 >> raport.txt
	./zadanie3 0.0000000001 8 >> raport.txt
	./zadanie3 0.0000000001 16 >> raport.txt

clean:
	rm zadanie3 integrate