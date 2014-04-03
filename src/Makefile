CFLAGS=-Wall -Wextra -O0 -g -ggdb -std=c99 -pedantic

all: util

%.o: %.c
	gcc $(CFLAGS) -c $<

util: util.c database.o bitmap.o
	gcc $(CFLAGS) -o $@ $^

clean:
	rm -f *.o
	rm -f util
