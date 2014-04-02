TESTS=test-bitmap test-database

all: $(TESTS)

CFLAGS=-Wall -Wextra -O0 -g -ggdb -pedantic

%.o: %.c
	gcc $(CFLAGS) -o $@ $<

test-%: ../src/%.c %.c
	gcc $(CFLAGS) $(SRC) -o $@ $^

clean:
	rm -f $(TESTS)
