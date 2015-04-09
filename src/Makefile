CFLAGS=-Wall -Wextra -O2 -g -std=c99 -pedantic -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security
CPPFLAGS=-D_FORTIFY_SOURCE=2
SONAME=libimgdup.so.3
LDFLAGS=-Wl,-soname,$(SONAME) -fPIC
PREFIX ?= /usr/local

all: libs utils

%.o: %.c
	gcc $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -c $<

libimgdup.so: database.o bitmap.o
	gcc $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -shared -o $(SONAME) $^
	ln -sf $(SONAME) libimgdup.so

libimgdup.a: database.o bitmap.o
	ar rcs libimgdup.a $^

libs: libimgdup.so libimgdup.a

imdb-query: imdb-query.o libimgdup.so
	gcc $(CFLAGS) $(CPPFLAGS) -L. $< -o $@ -limgdup

imdb-query-st: imdb-query.o libimgdup.a
	gcc $(CFLAGS) $(CPPFLAGS) -L. $^ -o $@ -static

imdb-1to2: imdb-1to2.c
	gcc $(CFLAGS) $(CPPFLAGS) -o $@ $^

utils: imdb-query imdb-query-st imdb-write imdb-1to2

GMFLAGS=$(shell GraphicsMagickWand-config --cppflags --ldflags --libs)
imdb-write: imdb-write.c sample.c bitmap.c database.c
	gcc $(CFLAGS) $(CPPFLAGS) $(GMFLAGS) -limgdup -o $@ $^

install: libimgdup.so utils
	install -m755 -s -o root imdb-query   $(PREFIX)/bin
	install -m755 -s -o root imdb-write   $(PREFIX)/bin
	install -m755 -s -o root imdb-1to2    $(PREFIX)/bin
	install -m644 -s -o root $(SONAME)    $(PREFIX)/lib
	install -m644 -s -o root libimgdup.so $(PREFIX)/lib

clean:
	rm -f *.o
	rm -f imdb-1to2
	rm -f imdb-query
	rm -f imdb-query-st
	rm -f imdb-write
	rm -f libimgdup*
