CFLAGS=-Wall -Wextra -O2 -g -std=c99 -pedantic -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security
CPPFLAGS=-D_FORTIFY_SOURCE=2
LDFLAGS=-Wl,-soname,libimgdup.so.1 -fPIC

all: lib util imdb_1to2

%.o: %.c
	gcc $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -c $<

lib: database.o bitmap.o
	gcc $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -shared -o libimgdup.so $^
	ar rcs libimgdup.a $^

util: util.c lib
	gcc $(CFLAGS) $(CPPFLAGS) -L. $< -limgdup    -o $@
	gcc $(CFLAGS) $(CPPFLAGS) -L. $< libimgdup.a -o $@-static -static

imdb_1to2: imdb_1to2.c
	gcc $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -f *.o
	rm -f imdb_?to?
	rm -f util
	rm -f util-static
	rm -f libimgdup*
