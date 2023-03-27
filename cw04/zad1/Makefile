CC = gcc
CFLAGS = -Wall

all:
	$(CC) $(CFLAGS) zadanie1.c -o zadanie1

test: all
	@echo "TESTING ignore \n-------------"
	./zadanie1 "ignore"
	@echo "\nTESTING handler\n-------------"
	./zadanie1 "handler"
	@echo "\nTESTING mask\n-------------"
	./zadanie1 "mask"
	@echo "\nTESTING pending\n-------------"
	./zadanie1 "pending"
