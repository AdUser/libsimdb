CFLAGS=-Wall -Wextra -O0 -g -ggdb -pedantic

all: util

%.o: %.c
	gcc $(CFLAGS) -c $<

util: main.c database.o bitmap.o
	gcc $(CFLAGS) -o $@ $^

clean:
	rm -f *.o
	rm -f util
