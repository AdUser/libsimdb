#ifndef HAS_BITMAP_H

#define BITMAP_SIZE 32

typedef unsigned char bitmap_t[BITMAP_SIZE];

int bitmap_compare(const unsigned char *a, const unsigned char *b);

#endif
#define HAS_BITMAP_H 1
