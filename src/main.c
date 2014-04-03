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
#include "database.h"
#include "bitmap.h"

int db_search(db_t *db, rec_t *sample, float tresh, match_t **matches)
{
  const int blk_size = 4096;
  uint64_t found = 0;
  block_t blk;
  unsigned int i = 0;
  unsigned char *p, *t;
  float diff;

  assert(db      != NULL);
  assert(sample  != NULL);
/*assert(matches != NULL);*/

  memset(&blk,    0x0, sizeof(block_t));
  blk.start = 1;
  blk.records = blk_size;

  if (db_rd_rec(db, sample) != 1)
    return -1;
  /* TODO: expand matches */

  while (db_rd_blk(db, &blk) > 0) {
    p = blk.data;
    for (i = 0; i < blk.records; i++, p += REC_LEN) {
      t = p + OFF_USED;
      if (*t == 0x0) continue;
      t = p + OFF_BITMAP;
      diff  = (float) bitmap_compare(t, sample->data + OFF_BITMAP);
      diff /= BITMAP_BITS;
      if (diff > tresh) continue;
      printf("%ld -- %f\n", blk.start + i, diff);
      found++;
    }
    FREE(blk.data);
    blk.start += blk_size;
  }

  return found;
}

int main(int argc, char **argv)
{
  db_t db;
  rec_t sample;

  if (argc < 3) {
    printf("Usage: test <path> <int>\n");
    exit(EXIT_FAILURE);
  }

  memset(&db,     0x0, sizeof(db_t));
  memset(&sample, 0x0, sizeof(rec_t));
  sample.num = atoll(argv[2]);

  assert(sample.num > 0);

  if (db_open(&db, argv[1]) == -1) {
    printf("%s\n", db.errstr);
    exit(EXIT_FAILURE);
  }

  db_search(&db, &sample, 0.15, NULL);

  db_close(&db);

  exit(EXIT_SUCCESS);
}
