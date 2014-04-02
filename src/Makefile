CFLAGS=-Wall -Wextra -O0 -g -ggdb -pedantic

all: *.o

%.o: %.c
	gcc -O0 -Wall -Wextra -pedantic -ggdb $< -o $@

clean:
	rm -f *.o
