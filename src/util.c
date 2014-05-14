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

#include "common.h"
#include "database.h"
#include "bitmap.h"

#include <unistd.h>
#include <getopt.h>

void usage(int exitcode) {
    printf("Usage:\n"
"  util <opts>\n"
"  -b <path>   Path to database\n"
"  -t <float>  Maximum difference percent (0.0 - 1.0, default: 0.1)\n"
);
    printf("\n"
"  -B <num>    Show bitmap for this sample\n"
"  -D <a>,<b>  Show difference bitmap for this samples\n"
"  -S <num>    Search for images similar ot this sample\n"
"  -U <num>    Show db usage map, <num> entries per column\n"
);
    exit(exitcode);
}

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

  if (db_rd_rec(db, sample) < 1)
    return -1;

  if (!sample->data[0]) {
    puts("Sample not exists");
    return 0;
  }
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
      printf("%lli -- %.2f\n", blk.start + i, diff * 100);
      found++;
    }
    FREE(blk.data);
    blk.start += blk_size;
  }

  return found;
}

int db_usage_map(db_t *db, unsigned short int cols)
{
  const int blk_size = 4096;
  unsigned int i, j = 0;
  unsigned char *p, *t;
  char buf[256];
  block_t blk;

  assert(db   != NULL);
  assert(cols <= 256);

  memset(&blk, 0x0, sizeof(block_t));
  memset(buf,  0x0, sizeof(char) * 256);

  blk.start = 1;
  blk.records = blk_size;

  while (db_rd_blk(db, &blk) > 0) {
    p = blk.data;
    for (i = 0; i < blk.records; i++, p += REC_LEN) {
      t = p + OFF_USED;
      buf[j] = (*t == 0xFF) ? '1' : '0';
      if (j++ < cols)
        continue;
      puts(buf);
      memset(buf,  0x0, sizeof(char) * 256);
      j = 0;
    }
    if (j > 0)
      puts(buf);
    FREE(blk.data);
    blk.start += blk_size;
  }

  return 0;
}

int rec_bitmap(db_t *db, rec_t *sample)
{
  uint16_t row;
  uint8_t i, j;
  assert(db      != NULL);
  assert(sample  != NULL);

  if (db_rd_rec(db, sample) < 1)
    return -1;

  if (!sample->data[0]) {
    puts("Sample not exists");
    return 0;
  }

  for (i = 0; i < 16; i++) {
    row = *(((uint16_t *) (&sample->data[OFF_BITMAP])) + i);
    for (j = 0; j < 16; j++) {
      putchar((row & 1) == 1 ? '1' : '0');
      row >>= 1;
    }
    putchar('\n');
  }

  return 0;
}

int rec_diffmap(db_t *db, unsigned long a, unsigned long b)
{
  rec_t src;
  rec_t dst;
  unsigned char diff[BITMAP_SIZE];
  uint16_t row;
  uint8_t i, j;

  assert(db != NULL);

  memset(&src, 1, sizeof(rec_t));
  memset(&dst, 2, sizeof(rec_t));

  src.num = a;
  dst.num = b;

  if (db_rd_rec(db, &src) < 1)
    return -1;

  if (!src.data[0]) {
    puts("First sample not exists");
    return 0;
  }

  if (db_rd_rec(db, &dst) < 1)
    return -1;

  if (!src.data[0]) {
    puts("First sample not exists");
    return 0;
  }

  bitmap_diffmap(&diff[0], src.data + OFF_BITMAP, dst.data + OFF_BITMAP);

  for (i = 0; i < 16; i++) {
    row = *(((uint16_t *) diff) + i);
    for (j = 0; j < 16; j++) {
      putchar((row & 1) == 1 ? '1' : '0');
      row >>= 1;
    }
    putchar('\n');
  }

  return 0;
}

int main(int argc, char **argv)
{
  enum { undef, search, bitmap, usage_map, diffmap } mode = undef;
  const char *db_path = NULL;
  float tresh = 0.10;
  unsigned short int cols = 64;
  db_t db;
  rec_t sample;
  unsigned long a, b;
  char *c = NULL;
  char opt = '\0';

  memset(&db,     0x0, sizeof(db_t));
  memset(&sample, 0x0, sizeof(rec_t));

  if (argc < 3)
    usage(EXIT_FAILURE);

  while ((opt = getopt(argc, argv, "b:t:B:D:S:U:")) != -1) {
    switch (opt) {
      case 'b' :
        db_path = optarg;
        break;
      case 't' :
        tresh = atof(optarg);
        tresh = (tresh > 0.0 && tresh < 1.0) ? tresh : 0.10;
        break;
      case 'B' :
        mode = bitmap;
        sample.num = atoll(optarg);
        break;
      case 'D' :
        mode = diffmap;
        if ((c = strchr(optarg, ',')) == NULL)
          usage(EXIT_FAILURE);
        a = atoll(optarg);
        b = atoll(c + 1);
        break;
      case 'S' :
        mode = search;
        sample.num = atoll(optarg);
        break;
      case 'U' :
        mode = usage_map;
        cols = atoi(optarg);
        if (cols <= 0 || cols >= 256)
          cols = 64;
        break;
      default :
        usage(EXIT_FAILURE);
        break;
    }
  }

  if ((mode == search || mode == bitmap) && sample.num <= 0)
    usage(EXIT_FAILURE);

  if (mode == diffmap && (a <= 0 || b <= 0))
    usage(EXIT_FAILURE);

  if (db_open(&db, db_path) == -1) {
    printf("%s\n", db.errstr);
    exit(EXIT_FAILURE);
  }

  if (mode == search)
    db_search(&db, &sample, tresh, NULL);

  if (mode == bitmap)
    rec_bitmap(&db, &sample);

  if (mode == usage_map)
    db_usage_map(&db, cols);

  if (mode == diffmap)
    rec_diffmap(&db, a, b);

  db_close(&db);

  exit(EXIT_SUCCESS);
}
