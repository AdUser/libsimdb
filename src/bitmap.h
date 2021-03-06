/* Copyright 2014-2017 Alex 'AdUser' Z (ad_user@runbox.com)
 *
 * This file is part of libsimdb
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef HAS_BITMAP_H
#define HAS_BITMAP_H 1

/**
 * @file
 * @brief Functions for work with image bitmaps
 */

/** Bits per bitmap side (currently - 16) */
#define SIMDB_BITMAP_SIDE 16
/** Total bits in bitmap (currently - 256) */
#define SIMDB_BITMAP_BITS (SIMDB_BITMAP_SIDE * SIMDB_BITMAP_SIDE)
/** Bitmap size in bytes (currently - 32) */
#define SIMDB_BITMAP_SIZE (SIMDB_BITMAP_BITS / 8)

/**
 * @brief Compare two bitmaps
 * @param a First bitmap to compare
 * @param b Second bitmap to compare
 * @returns Integer showing difference between bitmaps in bits (0-256)
 */
int simdb_bitmap_compare(const unsigned char *a, const unsigned char *b);

/**
 * @brief Unpack BITmap to BYTEmap
 * @param map Source bitmap
 * @param buf Pointer to store generated bytemap
 * @returns Size of generated bytemap (now is @a BITMAP_BITS)
 */
size_t simdb_bitmap_unpack(const unsigned char *map, char **buf);
#endif
