all: filter

CC=gcc
CFLAGS=-g -Wall

filter: filter.o Util.o
	$(CC) $^ -o $@

filter.o: filter.c filter.h Util.h
	$(CC) -c $(CFLAGS) $<

Util.o: Util.c Util.h
	$(CC) -c $(CFLAGS) $<

clean: 
	rm -f filter *.o