#ifndef HAS_BITMAP_H
#define HAS_BITMAP_H 1

#define BITMAP_SIDE 16
#define BITMAP_SIZE (BITMAP_SIDE * (BITMAP_SIDE / 8))
#define BITMAP_BITS (BITMAP_SIZE * 8)

#define CHAR_USED '@'
#define CHAR_NONE '-'

typedef unsigned char bitmap_t[BITMAP_SIZE];

int
bitmap_compare(const unsigned char *a,
               const unsigned char *b);

int
bitmap_diffmap(unsigned char *diff,
               const unsigned char *a,
               const unsigned char *b);

size_t
bitmap_unpack(const unsigned char *map,
              unsigned char ** const buf);

void
bitmap_print(const unsigned char *map);
#endif
