/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#include "main.h"
#include "image.h"

#include <gd.h>

int bitmap_compare(image_bitmap * const a, image_bitmap * const b)
{
  uint16_t row = 0;
  uint16_t bit = 0;
  size_t  cnt = 0;

  for (row = 0; row < 16; row++) {
    bit = 1 << 15;
    while (bit) {
      cnt += !!(((*a)[row] & bit) ^ ((*b)[row] & bit));
      bit >>= 1;
    }
  }

  return cnt;
}

int image_load(void)
{
  return 1;
}
