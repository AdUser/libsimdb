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

#define CHAR_USED '@'
#define CHAR_NONE '-'

void usage(int exitcode) {
    printf(
"Usage: imdb-util <opts>\n"
"  -b <path>   Path to database\n"
"  -t <float>  Maximum difference percent (0.0 - 1.0, default: 0.1)\n"
);
    printf("\n"
"  -I          Create database (init)\n"
"  -B <num>    Show bitmap for this sample\n"
"  -C <a>,<b>  Show difference percent for this samples\n"
"  -D <a>,<b>  Show difference bitmap for this samples\n"
"  -S <num>    Search for images similar ot this sample\n"
"  -U <num>    Show db usage map, <num> entries per column\n"
);
    exit(exitcode);
}

int search_similar(imdb_t *db, imdb_rec_t *sample, float tresh)
{
  int ret = 0, i = 0;
  imdb_match_t *matches = NULL;
  imdb_search_t search;
  memset(&search, 0x0, sizeof(search));
  search.tresh_ratio  = tresh;
  search.tresh_bitmap = tresh;

  ret = imdb_search(db, sample, &search, &matches);
  if (ret == -1) {
    printf("%s\n", db->errstr);
    exit(EXIT_FAILURE);
  }

  if (ret == 0)
    return 0;

  for (i = 0; i < ret; i++) {
    printf("%llu -- %.2f\n", matches[i].num, matches[i].diff * 100);
  }
  FREE(matches);
  return 0;
}

int db_usage_map(imdb_t *db, unsigned short int cols)
{
  char *map = NULL;
  char *m   = NULL;
  char row[256];
  uint64_t records;
  uint8_t rest = 0;

  memset(row, 0x0, sizeof(char) * 256);

  records = imdb_records_count(db);
  CALLOC(map, records + 1, sizeof(char));

  if ((records = imdb_usage_map(db, map)) == 0) {
    printf("Cant get database usage map\n");
    FREE(map);
    exit(1);
  }

  m = map;
  while (records) {
    rest = (records > cols) ? cols : records;
    memcpy(row, m, rest);
    printf("%7d : %s\n", (m - map + 1), row);
    m       += rest;
    records -= rest;
  }

  FREE(map);

  return 0;
}

int rec_bitmap(imdb_t *db, imdb_rec_t *sample)
{
  uint16_t row;
  uint8_t i, j;
  char c;
  assert(db      != NULL);
  assert(sample  != NULL);

  if (imdb_read_rec(db, sample) < 1)
    return -1;

  if (!sample->data[0]) {
    puts("Sample not exists");
    return 0;
  }

  for (i = 0; i < 16; i++) {
    row = *(((uint16_t *) (&sample->data[REC_OFF_BM])) + i);
    for (j = 0; j < 16; j++) {
      c = (row & 1) == 1 ? CHAR_USED : CHAR_NONE;
      putchar(c);
      putchar(c);
      row >>= 1;
    }
    putchar('\n');
  }

  return 0;
}

int rec_diff(imdb_t *db, unsigned long a, unsigned long b, unsigned short int showmap)
{
  imdb_rec_t src;
  imdb_rec_t dst;
  float diff = 0.0;
  unsigned char map[BITMAP_SIZE];
  uint16_t row;
  uint8_t i, j;

  assert(db != NULL);

  memset(&src, 1, sizeof(imdb_rec_t));
  memset(&dst, 2, sizeof(imdb_rec_t));

  src.num = a;
  dst.num = b;

  if (imdb_read_rec(db, &src) < 1)
    return -1;

  if (!src.data[0]) {
    puts("First sample not exists");
    return 0;
  }

  if (imdb_read_rec(db, &dst) < 1)
    return -1;

  if (!src.data[0]) {
    puts("First sample not exists");
    return 0;
  }

  if (showmap == 0) {
    diff  = (float) bitmap_compare(&src.data[REC_OFF_BM], &dst.data[REC_OFF_BM]);
    diff /= BITMAP_BITS;
    printf("%.2f%%\n", diff * 100);
    return 0;
  }

  bitmap_diffmap(&map[0], src.data + REC_OFF_BM, dst.data + REC_OFF_BM);

  for (i = 0; i < 16; i++) {
    row = *(((uint16_t *) map) + i);
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
  enum { undef, init, search, bitmap, usage_map, diff } mode = undef;
  const char *db_path = NULL;
  float tresh = 0.10;
  unsigned short int cols = 64, map = 0;
  imdb_t db;
  imdb_rec_t sample;
  unsigned long a = 0, b = 0;
  char *c = NULL;
  char opt = '\0';

  memset(&db,     0x0, sizeof(imdb_t));
  memset(&sample, 0x0, sizeof(imdb_rec_t));

  if (argc < 3)
    usage(EXIT_FAILURE);

  while ((opt = getopt(argc, argv, "b:t:IB:C:D:S:U:")) != -1) {
    switch (opt) {
      case 'b' :
        db_path = optarg;
        break;
      case 't' :
        tresh = atof(optarg);
        tresh = (tresh > 0.0 && tresh < 1.0) ? tresh : 0.10;
        break;
      case 'I' :
        mode = init;
        break;
      case 'B' :
        mode = bitmap;
        sample.num = atoll(optarg);
        break;
      case 'D' :
        map = 1;
      case 'C' :
        mode = diff;
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

  if (mode == diff && (a <= 0 || b <= 0))
    usage(EXIT_FAILURE);

  if (mode == init) {
    if (imdb_init(&db, db_path) == -1) {
      printf("database init: %s\n", db.errstr);
      exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
  }

  if (imdb_open(&db, db_path) == -1) {
    printf("database open: %s\n", db.errstr);
    exit(EXIT_FAILURE);
  }

  if (mode == search)
    search_similar(&db, &sample, tresh);

  if (mode == bitmap)
    rec_bitmap(&db, &sample);

  if (mode == usage_map)
    db_usage_map(&db, cols);

  if (mode == diff)
    rec_diff(&db, a, b, map);

  imdb_close(&db);

  exit(EXIT_SUCCESS);
}
