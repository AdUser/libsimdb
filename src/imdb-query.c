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
    puts(
"Usage: imdb-query <opts>\n"
"  -b <path>   Path to database\n"
"  -t <int>    Maximum difference pct (0 - 50, default: 10%)\n"
);
    puts(
"  -B <num>    Show bitmap for this sample\n"
"  -C <a>,<b>  Show difference percent for this samples\n"
"  -D <a>,<b>  Show difference bitmap for this samples\n"
"  -S <num>    Search for images similar ot this sample\n"
"  -U <num>    Show db usage map, <num> entries per column\n"
"              Special case - 0, output will be single line\n"
);
    exit(exitcode);
}

int search_similar(imdb_db_t *db, uint64_t number, float maxdiff)
{
  int ret = 0, i = 0;
  imdb_rec_t sample;
  imdb_match_t *matches = NULL;
  imdb_search_t search;

  memset(&sample, 0x0, sizeof(imdb_rec_t));
  memset(&search, 0x0, sizeof(imdb_search_t));
  search.maxdiff_ratio  = 0.2; /* 20% */
  search.maxdiff_bitmap = maxdiff;

  sample.num = number;
  if ((ret = imdb_search(db, &sample, &search, &matches)) < 0) {
    puts(db->errstr);
    return 0;
  }

  for (i = 0; i < ret; i++) {
    printf("%llu -- %.1f (bitmap), %.1f (ratio)\n",
      matches[i].num,
      matches[i].diff_bitmap * 100,
      matches[i].diff_ratio  * 100);
  }

  FREE(matches);

  return 0;
}

int db_usage_map(imdb_db_t *db, unsigned short int cols)
{
  char *map = NULL;
  char *m   = NULL;
  char row[256];
  uint64_t records, pos;
  uint8_t rest = 0;

  memset(row, 0x0, sizeof(char) * 256);

  if ((records = imdb_usage_map(db, &map)) == 0) {
    printf("Can't get database usage map\n");
    FREE(map);
    return 0;
  }

  if (cols == 0) {
    putchar(CHAR_NONE); /* zero */
    puts(map);
    FREE(map);
    return 0;
  }

  m = map;
  while (records) {
    rest = (records > cols) ? cols : records;
    memcpy(row, m, rest);
    pos = m - map + 1;
    printf("%7llu : %s\n", pos, row);
    m       += rest;
    records -= rest;
  }

  FREE(map);
  return 0;
}

int rec_bitmap(imdb_db_t *db, uint64_t number)
{
  imdb_rec_t rec;

  assert(db != NULL);
  memset(&rec, 0x0, sizeof(imdb_rec_t));

  rec.num = number;
  if (imdb_read_rec(db, &rec) < 1) {
    puts("Sample not exists");
    return 0;
  }

  bitmap_print(&rec.data[REC_OFF_BM]);

  return 0;
}

int rec_diff(imdb_db_t *db, uint64_t a, uint64_t b, unsigned short int showmap)
{
  imdb_rec_t rec;
  float diff = 0.0;
  unsigned char one[BITMAP_SIZE];
  unsigned char two[BITMAP_SIZE];
  unsigned char map[BITMAP_SIZE];

  assert(db != NULL);
  memset(&rec, 0x0, sizeof(imdb_rec_t));

  rec.num = a;
  if (imdb_read_rec(db, &rec) < 1) {
    puts("First sample not exists");
    return 0;
  }
  memcpy(one, &rec.data[REC_OFF_BM], BITMAP_SIZE);

  rec.num = b;
  if (imdb_read_rec(db, &rec) < 1) {
    puts("Second sample not exists");
    return 0;
  }
  memcpy(two, &rec.data[REC_OFF_BM], BITMAP_SIZE);

  if (showmap) {
    bitmap_diffmap(map, one, two);
    bitmap_print(map);
    return 0;
  }

  diff = (float) bitmap_compare(one, two);
  printf("%.2f%%\n", (diff / BITMAP_BITS) * 100);

  return 0;
}

int main(int argc, char **argv)
{
  enum { undef, search, bitmap, usage_map, diff } mode = undef;
  const char *db_path = NULL;
  float maxdiff = 0.10;
  unsigned short int cols = 64, map = 0;
  imdb_db_t db;
  uint64_t a = 0, b = 0;
  char *c = NULL;
  char opt = '\0';

  memset(&db, 0x0, sizeof(imdb_db_t));

  if (argc < 3)
    usage(EXIT_FAILURE);

  while ((opt = getopt(argc, argv, "b:t:B:C:D:S:U:")) != -1) {
    switch (opt) {
      case 'b' :
        db_path = optarg;
        break;
      case 't' :
        maxdiff = atoi(optarg);
        if (maxdiff > 50 || maxdiff < 0) {
          puts("maxdiff out of bounds (0% - 50%), using default - 10%");
          maxdiff = 10;
        }
        maxdiff /= 100;
        break;
      case 'B' :
        mode = bitmap;
        a = atoll(optarg);
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
        a = atoll(optarg);
        break;
      case 'U' :
        mode = usage_map;
        cols = atoi(optarg);
        if (cols >= 256)
          cols = 100;
        break;
      default :
        usage(EXIT_FAILURE);
        break;
    }
  }

  if (db_path == NULL) {
    puts("database path not set");
    usage(EXIT_FAILURE);
  }

  if (imdb_open(&db, db_path, 0) == -1) {
    printf("database open: %s\n", db.errstr);
    exit(EXIT_FAILURE);
  }

  switch (mode) {
    case search :
      if (a <= 0) {
        puts("can't parse number");
        usage(EXIT_FAILURE);
      }
      search_similar(&db, a, maxdiff);
      break;
    case bitmap :
      if (a <= 0) {
        puts("can't parse number");
        usage(EXIT_FAILURE);
      }
      rec_bitmap(&db, a);
      break;
    case usage_map :
      db_usage_map(&db, cols);
      break;
    case diff :
      if (a <= 0 || b <= 0) {
        puts("both numbers must be set");
        exit(EXIT_FAILURE);
      }
      rec_diff(&db, a, b, map);
      break;
    default :
      usage(EXIT_SUCCESS);
      break;
  }

  imdb_close(&db);

  exit(EXIT_SUCCESS);
}
