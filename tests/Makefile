all: test-bitmap

CFLAGS=-Wall -Wextra -O0 -g -ggdb -pedantic

%.o: %.c
	gcc $(CFLAGS) $< -o $@

SRC=../src/bitmap.c bitmap.c
test-bitmap:
	gcc $(CFLAGS) $(SRC) -o $@

clean:
	rm -f test-bitmap
