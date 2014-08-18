CFLAGS=-Wall -Wextra -O0 -g -ggdb -std=c99 -pedantic

all: util

%.o: %.c
	gcc $(CFLAGS) -c $<

util: util.c database.o bitmap.o
	gcc $(CFLAGS)         -o $@ $^
	gcc $(CFLAGS) -static -o $@-static $^

clean:
	rm -f *.o
	rm -f util
	rm -f util-static
