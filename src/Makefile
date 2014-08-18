CFLAGS=-Wall -Wextra -O0 -g -std=c99 -pedantic -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security
CPPFLAGS=-D_FORTIFY_SOURCE=2
LDFLAGS=-shared -fPIC -Wl,-z,relro -L.

all: lib util

%.o: %.c
	gcc $(CFLAGS) $(CPPFLAGS) -c $<

lib: database.o bitmap.o
	gcc $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -Wl,-soname,libimgdup.so.1 -o libimgdup.so $^

util: util.c
	gcc $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -limgdup         -o $@ $^
	gcc $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -limgdup -static -o $@-static $^

clean:
	rm -f *.o
	rm -f util
	rm -f util-static
	rm -f libimgdup*
