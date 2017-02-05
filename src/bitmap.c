/* Copyright 2014-2017 Alex 'AdUser' Z (ad_user@runbox.com)
 *
 * This file is part of libsimdb
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/**
 * @file
 * @brief Functions for work with image bitmaps
 */

#include "common.h"
#include "bitmap.h"

/** bictionary for speedup bitmap comparing */
static unsigned char dict[256] = {
/* 0x00 _0 _1 _2 _3 _4 _5 _6 _7 _8 _9 _A _B _C _D _E _F */
/* ---------------------------------------------------- */
/* 0_ */ 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
/* 1_ */ 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
/* 2_ */ 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
/* 3_ */ 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
/* 4_ */ 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
/* 5_ */ 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
/* 6_ */ 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
/* 7_ */ 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
/* 8_ */ 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
/* 9_ */ 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
/* A_ */ 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
/* B_ */ 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
/* C_ */ 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
/* D_ */ 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
/* E_ */ 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
/* F_ */ 4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,
};

int
simdb_bitmap_compare(const unsigned char *a, const unsigned char *b) {
  unsigned char diff = 0;
  size_t i = 0;
  size_t cnt = 0;

  for (i = 0; i < SIMDB_BITMAP_SIZE; i++, a++, b++) {
    diff = *a ^ *b;
    cnt += dict[diff];
  }

  return cnt;
}

size_t
simdb_bitmap_unpack(const unsigned char *map, char **buf) {
  size_t buf_size = SIMDB_BITMAP_BITS;
  uint16_t *p, row, mask;
  char *q = NULL;

  assert(map != NULL);
  assert(buf != NULL);

  if ((*buf = calloc(buf_size, sizeof(char))) == NULL)
    return 0;

  p = (uint16_t *) map;
  q = *buf;
  for (size_t i = 0; i < SIMDB_BITMAP_SIDE; i++, p++) {
    row = *p; mask = 0x1;
    for (size_t j = 0; j < SIMDB_BITMAP_SIDE; j++, q++) {
      *q = (row & mask) ? 0x1 : 0x0;
      mask <<= 1;
    }
  }

  return buf_size;
}
