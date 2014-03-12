all: test-bitmap

%.o: %.c
	gcc -O0 -Wall -Wextra -pedantic -ggdb $< -o $@

test-bitmap: test-bitmap.c bitmap.c
	gcc -Wall -Wextra -pedantic -g -ggdb test-bitmap.c bitmap.c -o test-bitmap

clean:
	rm -f *.o
	rm -f test-bitmap
