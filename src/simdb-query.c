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
#include "bitmap.h"
#include "record.h"
#include "io.h"
#include "simdb.h"

#include <unistd.h>
#include <getopt.h>

#define CHAR_USED '@'
#define CHAR_FREE '-'

void usage(int exitcode) {
  fprintf(stderr,
"Usage: simdb-query <opts>\n"
"  -b <path>   Path to database\n"
"  -t <int>    Maximum difference pct (0 - 50, default: 10%%)\n"
);
  fprintf(stderr,
"  -B <num>    Show bitmap for this sample\n"
"  -C <a>,<b>  Show difference percent for this samples\n"
"  -D <a>,<b>  Show difference bitmap for this samples\n"
"  -S <num>    Search for images similar ot this sample\n"
"  -U <num>    Show db usage map, <num> entries per column\n"
"              Special case - 0, output will be single line\n"
"  -W <a>,<b>  Show usage map starting from <a>, but no more\n"
"              than <b> entries (limit)\n"
);
  exit(exitcode);
}

/**
 * @brief Print bitmap to stdout as ascii-square
 * @param map Source bitmap
 * @note Height of "square" is equals to BITMAP_SIDE,
 *   but width is BITMAP_SIDE x 2, for ease of reading
 */
void
bitmap_print(const char *map, size_t side) {
  const char *p = NULL;
  char line[side * 2 + 1];

  assert(map != NULL);

  p = map;
  for (size_t i = 0; i < side; i++) {
    line[side * 2] = '\0';
    for (size_t j = 0; j < side; j++, p++) {
      line[(j * 2) + 0] = *p ? CHAR_USED : CHAR_FREE;
      line[(j * 2) + 1] = *p ? CHAR_USED : CHAR_FREE;
    }
    puts(line);
  }
}

int search_similar(simdb_t *db, int num, float maxdiff) {
  int ret = 0, i = 0;
  simdb_search_t search;

  memset(&search, 0x0, sizeof(simdb_search_t));

  search.maxdiff_ratio  = 0.2; /* 20% */
  search.maxdiff_bitmap = maxdiff;

  if ((ret = simdb_search_byid(db, &search, num)) < 0) {
    fprintf(stderr, "%s\n", simdb_error(ret));
    return 1;
  }

  for (i = 0; i < search.found; i++) {
    printf("%llu -- %.1f (bitmap), %.1f (ratio)\n",
      search.matches[i].num,
      search.matches[i].diff_bitmap * 100,
      search.matches[i].diff_ratio  * 100);
  }

  if (search.found > 0)
    FREE(search.matches);

  return 0;
}

int db_usage_map(simdb_t *db, unsigned short int cols)
{
  char *map = NULL;
  char *m   = NULL;
  char row[256];
  uint64_t records, pos;
  uint8_t rest = 0;

  memset(row, 0x0, sizeof(char) * 256);

  if ((records = simdb_usage_map(db, &map)) == 0) {
    fprintf(stderr, "database usage: can't get database map\n");
    FREE(map);
    return 1;
  }

  for (uint16_t i = 0; i < records; i++)
    map[i] = map[i] ? CHAR_USED : CHAR_FREE;

  if (cols == 0) {
    putchar(CHAR_FREE); /* zero */
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

int db_usage_slice(simdb_t *db, uint64_t offset, uint16_t limit)
{
  char *map = NULL;

  limit = simdb_usage_slice(db, &map, offset, limit);
  for (uint16_t i = 0; i < limit; i++)
    map[i] = map[i] ? CHAR_USED : CHAR_FREE;
  puts(map);
  FREE(map);

  return 0;
}

int rec_bitmap(simdb_t *db, int num) {
  char *bitmap;
  size_t side;
  int ret;

  assert(db != NULL);

  if ((ret = simdb_record_bitmap(db, num, &bitmap, &side)) < 0) {
    fprintf(stderr, "can't get record's bitmap: %s\n", simdb_error(ret));
    return 1;
  } else if (ret == 0) {
    fprintf(stderr, "bitmap: %s\n", "sample not found");
    return 1;
  }

  bitmap_print(bitmap, side);
  free(bitmap);

  return 0;
}

int rec_diff(simdb_t *db, int a, int b, unsigned short int showmap) {
  char *map1 = NULL, *map2 = NULL, *dmap = NULL;
  size_t size = 0, diff = 0;
  int ret;

  assert(db != NULL);

  do {
    if ((ret = simdb_record_bitmap(db, a, &map1, &size)) < 0) {
      if (ret < 0) {
        fprintf(stderr, "can't get bitmap for record #%d: %s\n", a, simdb_error(ret));
      } else {
        fprintf(stderr, "record diff: record #%d not exists\n", a);
      }
      ret = 1;
      break;
    }

    if ((ret = simdb_record_bitmap(db, b, &map2, &size)) <= 0) {
      if (ret < 0) {
        fprintf(stderr, "can't get bitmap for record #%d: %s\n", b, simdb_error(ret));
      } else {
        fprintf(stderr, "record diff: record #%d not exists\n", b);
      }
      ret = 1;
      break;
    }

    if ((dmap = calloc(size, sizeof(char))) == NULL) {
      ret = 1;
      break;
    }
    char *m1 = map1, *m2 = map2, *dm = dmap;
    for (size_t i = 0; i < size; i++, m1++, m2++, dm++) {
      *dm = *m1 ^ *m2;
      diff += *dm ? 0 : 1;
    }
    if (showmap) {
      bitmap_print(dmap, size);
    } else {
      printf("%.2f%%\n", ((float) diff / size) * 100);
    }
    ret = 0;
  } while (0);

  free(map1);
  free(map2);
  free(dmap);

  return ret;
}

int main(int argc, char **argv)
{
  enum { undef, search, bitmap, usage_map, usage_slice, diff } mode = undef;
  const char *db_path = NULL;
  float maxdiff = 0.10;
  unsigned short int cols = 64, map = 0, ret = 0;
  simdb_t *db = NULL;
  uint64_t a = 0, b = 0;
  char *c = NULL;
  int err;
  char opt = '\0';

  if (argc < 3)
    usage(EXIT_FAILURE);

  while ((opt = getopt(argc, argv, "b:t:B:C:D:S:U:W:")) != -1) {
    switch (opt) {
      case 'b' :
        db_path = optarg;
        break;
      case 't' :
        maxdiff = atoi(optarg);
        if (maxdiff > 50 || maxdiff < 0) {
          fprintf(stderr, "maxdiff out of bounds (0%% - 50%%), using default - 10%%\n");
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
      case 'W' :
        mode = usage_slice;
        if ((c = strchr(optarg, ',')) == NULL)
          usage(EXIT_FAILURE);
        a = atoll(optarg);
        b = atoll(c + 1);
        break;
      default :
        usage(EXIT_FAILURE);
        break;
    }
  }

  if (db_path == NULL) {
    fprintf(stderr, "database path not set\n");
    exit(EXIT_FAILURE);
  }

  if ((db = simdb_open(db_path, 0, &err)) == NULL) {
    fprintf(stderr, "database open: %d\n", err);
    exit(EXIT_FAILURE);
  }

  switch (mode) {
    case search :
      if (a <= 0) {
        fprintf(stderr, "can't parse number\n");
        usage(EXIT_FAILURE);
      }
      ret = search_similar(db, a, maxdiff);
      break;
    case bitmap :
      if (a <= 0) {
        fprintf(stderr, "can't parse number\n");
        usage(EXIT_FAILURE);
      }
      ret = rec_bitmap(db, a);
      break;
    case usage_map :
      ret = db_usage_map(db, cols);
      break;
    case usage_slice :
      ret = db_usage_slice(db, a, b);
      break;
    case diff :
      if (a <= 0 || b <= 0) {
        fprintf(stderr, "both numbers must be set\n");
        exit(EXIT_FAILURE);
      }
      ret = rec_diff(db, a, b, map);
      break;
    default :
      usage(EXIT_SUCCESS);
      break;
  }

  simdb_close(db);

  return ret;
}
