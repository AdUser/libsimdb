CFLAGS=-Wall -Wextra -O0 -g -ggdb -pedantic

all: test-image

%.c: %.h

%.o: %.c
	gcc -O0 -Wall -Wextra -pedantic -ggdb $< -o $@

clean:
	rm -f *.o
	rm -f test-image

test-image: image.c test-image.c
	gcc $(CFLAGS) -lmagic -lgd image.c test-image.c -o $@
