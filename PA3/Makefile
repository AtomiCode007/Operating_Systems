CC = gcc

CFLAGS = -c -g -Wall -Wextra

LFLAGS = -Wall -Wextra -pthread


.PHONY: all clean


all: multi-lookup


multi-lookup: multi-lookup.o Queue.o
	$(CC) $(LFLAGS) $^ -o $@


multi-lookup.o: multi-lookup.c
	$(CC) $(CFLAGS) $<

Queue.o: Queue.c Queue.h
	$(CC) $(CFLAGS) $<



clean:

	rm -f multi-lookup

	rm -f *.o

	rm -f *~

	rm -f results.txt
	
	rm -f serviced.txt