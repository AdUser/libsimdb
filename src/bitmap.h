#ifndef HAS_BITMAP_H
#define HAS_BITMAP_H 1

#define BITMAP_SIZE 32
#define BITMAP_BITS 8 * BITMAP_SIZE

typedef unsigned char bitmap_t[BITMAP_SIZE];

int
bitmap_compare(const unsigned char *a,
               const unsigned char *b);

int
bitmap_diffmap(unsigned char *diff,
               const unsigned char *a,
               const unsigned char *b);
#endif
