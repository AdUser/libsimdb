CFLAGS=-Wall -Wextra -O2 -g -std=c99 -pedantic -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security
CPPFLAGS=-D_FORTIFY_SOURCE=2
SONAME=libimgdup.so.2
LDFLAGS=-Wl,-soname,$(SONAME) -fPIC

all: libs utils imdb_1to2

%.o: %.c
	gcc $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -c $<

libimgdup.so: database.o bitmap.o
	gcc $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -shared -o $(SONAME) $^
	ln -sf $(SONAME) libimgdup.so

libimgdup.a: database.o bitmap.o
	ar rcs libimgdup.a $^

libs: libimgdup.so libimgdup.a

util: util.o libimgdup.so
	gcc $(CFLAGS) $(CPPFLAGS) -L. $< -o $@ -limgdup

util-static: util.o libimgdup.a
	gcc $(CFLAGS) $(CPPFLAGS) -L. $^ -o $@ -static

utils: util util-static

imdb_1to2: imdb_1to2.c
	gcc $(CFLAGS) $(CPPFLAGS) -o $@ $^

FLAGS=$(shell GraphicsMagickWand-config --cppflags --ldflags --libs)
sampler-test: sampler.c bitmap.c sampler-test.c
	gcc $(CFLAGS) $(CPPFLAGS) $(FLAGS) -o $@ $^

clean:
	rm -f *.o
	rm -f imdb_?to?
	rm -f util
	rm -f util-static
	rm -f libimgdup*
