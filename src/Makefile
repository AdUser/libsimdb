CFLAGS=-Wall -Wextra -O2 -g -std=c99 -pedantic -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security
CPPFLAGS=-D_FORTIFY_SOURCE=2
SONAME=libimgdup.so.2
LDFLAGS=-Wl,-soname,$(SONAME) -fPIC

all: libs utils

%.o: %.c
	gcc $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -c $<

libimgdup.so: database.o bitmap.o
	gcc $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -shared -o $(SONAME) $^
	ln -sf $(SONAME) libimgdup.so

libimgdup.a: database.o bitmap.o
	ar rcs libimgdup.a $^

libs: libimgdup.so libimgdup.a

imdb-query: query.o libimgdup.so
	gcc $(CFLAGS) $(CPPFLAGS) -L. $< -o $@ -limgdup

imdb-query-st: query.o libimgdup.a
	gcc $(CFLAGS) $(CPPFLAGS) -L. $^ -o $@ -static

imdb_1to2: imdb_1to2.c
	gcc $(CFLAGS) $(CPPFLAGS) -o $@ $^

utils: imdb-query imdb-query-st imdb_1to2

GMFLAGS=$(shell GraphicsMagickWand-config --cppflags --ldflags --libs)
sampler-test: sampler.c bitmap.c sampler-test.c
	gcc $(CFLAGS) $(CPPFLAGS) $(GMFLAGS) -o $@ $^

clean:
	rm -f *.o
	rm -f imdb_?to?
	rm -f imdb-query
	rm -f imdb-query-st
	rm -f libimgdup*
