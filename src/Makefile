CFLAGS=-Wall -Wextra -O0 -g -ggdb -pedantic

all: test

%.o: %.c
	gcc $(CFLAGS) -c $<

test: main.c database.o bitmap.o
	gcc $(CFLAGS) -o $@ $^

clean:
	rm -f *.o
	rm -f test
