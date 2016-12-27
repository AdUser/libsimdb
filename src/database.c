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

#define DB_READ(db, buf, len, off) \
  errno = 0; \
  memset((buf), 0x0, (len)); \
  bytes = pread((db)->fd, (buf), (len), (off)); \
  if (errno) { \
    strncpy((db)->error, strerror(errno), sizeof(db->error)); \
    return -1; \
  }

#define DB_WRITE(db, buf, len, off) \
  errno = 0; \
  bytes = pwrite((db)->fd, (buf), (len), (off)); \
  if (errno) { \
    strncpy((db)->error, strerror(errno), sizeof(db->error)); \
    return -1; \
  }

const char *imdb_hdr_fmt = "IMDB v%02u, CAPS: %s;";

bool
imdb_create(const char *path) {
  ssize_t bytes = 0;
  unsigned char buf[IMDB_REC_LEN];
  const char *caps = "M-R";
  bool result = false;
  int fd = -1;

  memset(buf, 0x0, sizeof(char) * IMDB_REC_LEN);

  if ((fd = creat(path, 0644)) < 0)
    return result;

  snprintf((char *) buf, IMDB_REC_LEN, imdb_hdr_fmt, IMDB_VERSION, caps);

  bytes = pwrite(fd, buf, IMDB_REC_LEN, 0);
  if (bytes == IMDB_REC_LEN)
    result = true; /* success */

  close(fd);

  return result;
}

imdb_db_t *
imdb_open(const char *path, int mode, int *error) {
  imdb_db_t *db = NULL;
  ssize_t bytes = 0;
  struct stat st;
  char buf[IMDB_REC_LEN] = "\0";
  int flags = 0, fd = -1;
  char *p;

  assert(path  != NULL);
  assert(error != NULL);

  if (stat(path, &st) < 0) {
    *error = -1;
    return NULL;
  }

  if ((fd = open(path, (mode & IMDB_FLAG_WRITE) ? O_RDWR : O_RDONLY)) < 0) {
    *error = -1;
    return NULL;
  }

  errno = 0;
  bytes = pread(fd, buf, IMDB_REC_LEN, 0);
  if (bytes < IMDB_REC_LEN) {
    *error = errno ? -1 : -3; /* empty file, damaged database or IO error */
    return NULL;
  }

  p = buf + 0;
  if (memcmp("IMDB", p, 4) != 0) {
    *error = -3; /* Not a database file */
    return NULL;
  }

  p = buf + 6;
  if (atoi(p) != IMDB_VERSION) {
    *error = -4; /* Database version mismatch */
    return NULL;
  }

  p = buf + 10;
  if (memcmp("CAPS", p, 4) != 0) {
    *error = -3; /* Can't read database capabilities */
    return NULL;
  }

  if (mode & IMDB_FLAG_WRITE)
    flags |= IMDB_FLAG_WRITE;

  /* all seems to be ok */

  if ((db = calloc(1, sizeof(imdb_db_t))) == NULL) {
    *error = -2; /* out of memory */
    return NULL;
  }

  p = buf + 16;
  for (size_t i = 0; i < 8 && *p != '\0'; p++) {
    switch (*p) {
      case 'M' : flags |= IMDB_CAP_BITMAP; break;
      case 'C' : flags |= IMDB_CAP_COLORS; break;
      case 'R' : flags |= IMDB_CAP_RATIO;  break;
      case ';' : i = 9; /* end of flags */ break;
      default: /* ignore */ break;
    }
  }

  db->fd    = fd;
  db->flags = flags;

  strncpy(db->path, path, sizeof(db->path));

  return db;
}

void
imdb_close(imdb_db_t *db)
{
  assert(db != NULL);

  if (db->fd >= 0)
    close(db->fd);
}

int imdb_read_rec(imdb_db_t *db, imdb_rec_t *rec)
{
  ssize_t bytes = 0;

  assert(db  != NULL);
  assert(rec != NULL);
  assert(rec->num > 0);

  DB_READ(db, rec->data, IMDB_REC_LEN, rec->num * IMDB_REC_LEN);

  if (bytes != IMDB_REC_LEN)
    return -1;

  if (rec->data[0] != 0xFF)
    return 0;

  return 1;
}

int imdb_write_rec(imdb_db_t *db, imdb_rec_t *rec)
{
  ssize_t bytes = 0;

  assert(db  != NULL);
  assert(rec != NULL);
  assert(rec->num > 0);

  DB_WRITE(db, rec->data, IMDB_REC_LEN, rec->num * IMDB_REC_LEN);

  if (bytes != IMDB_REC_LEN)
    return -1;

  return 1;
}

int imdb_read_blk(imdb_db_t *db, imdb_block_t *blk)
{
  ssize_t bytes = 0;

  assert(db  != NULL);
  assert(blk != NULL);
  assert(blk->start > 0);
  assert(blk->records > 0);

  FREE(blk->data);
  CALLOC(blk->data, blk->records, IMDB_REC_LEN);
  DB_READ(db, blk->data, blk->records * IMDB_REC_LEN, blk->start * IMDB_REC_LEN);
  blk->records = bytes / IMDB_REC_LEN;

  return blk->records;
}

uint64_t
imdb_records_count(imdb_db_t * const db) {
  struct stat st;
  off_t size = 0;

  memset(&st, 0x0, sizeof(struct stat));

  fstat(db->fd, &st);
  size = st.st_size;

  return size / IMDB_REC_LEN;
}

float
ratio_from_rec_data(unsigned char * const data) {
  uint16_t iw, ih;

  iw = *((uint16_t *)(data + REC_OFF_IW));
  ih = *((uint16_t *)(data + REC_OFF_IH));

  return (iw > 0 && ih > 0) ? ((float) iw / ih) : 0.0;
}

int
imdb_search(imdb_db_t     * const db,
            imdb_rec_t    * const sample,
            imdb_search_t * const search,
            imdb_match_t  **matches)
{
  imdb_block_t blk;
  imdb_match_t match;
  const int blk_size = 4096;
  uint64_t found = 0;
  unsigned int i = 0;
  unsigned char *p = NULL;
  float ratio_s = 0.0; /* source */
  float ratio_t = 0.0; /* tested */

  assert(db      != NULL);
  assert(sample  != NULL);
  assert(search  != NULL);
  assert(matches != NULL);
  assert(search->maxdiff_ratio  >= 0.0 && search->maxdiff_ratio  <= 1.0);
  assert(search->maxdiff_bitmap >= 0.0 && search->maxdiff_bitmap <= 1.0);

  memset(&blk,   0x0, sizeof(imdb_block_t));
  memset(&match, 0x0, sizeof(imdb_match_t));
  blk.start = 1;
  blk.records = blk_size;

  if (imdb_read_rec(db, sample) < 1) {
    strncpy(db->error, "Can't read source sample", sizeof(db->error));
    return -1;
  }

  if (search->limit == 0)
    search->limit = -1; /* unsigned -> max */

  if (search->maxdiff_ratio > 0.0)
    ratio_s = ratio_from_rec_data(sample->data);

  CALLOC(*matches, search->limit, sizeof(imdb_match_t));

  while (imdb_read_blk(db, &blk) > 0) {
    p = blk.data;
    for (i = 0; i < blk.records; i++, p += IMDB_REC_LEN) {
      if (*(p + REC_OFF_RU) == 0x0)
        continue; /* record missing */

      match.diff_ratio  = 0.0;
      match.diff_bitmap = 0.0;

      /* - compare ratio - cheap */
      if (ratio_s > 0.0 && (ratio_t = ratio_from_rec_data(p)) > 0.0) {
        match.diff_ratio  =  ratio_s - ratio_t;
        match.diff_ratio *= (ratio_s > ratio_t) ? 1.0 : -1.0;
        if (match.diff_ratio > search->maxdiff_ratio)
          continue;
      } else {
        /* either source or target ratio not set, can't compare, skip test */
      }

      /* - compare bitmap - more expensive */
      match.diff_bitmap  = (float) bitmap_compare(p + REC_OFF_BM, sample->data + REC_OFF_BM);
      match.diff_bitmap /= BITMAP_BITS;
      if (match.diff_bitmap > search->maxdiff_bitmap)
        continue;

      /* create match record */
      match.num = blk.start + i;
      memcpy(&(*matches)[found], &match, sizeof(imdb_match_t));
      found++;
      if (found >= search->limit)
        break;
    }
    FREE(blk.data);
    if (found >= search->limit)
      break;
    blk.start += blk_size;
  }

  return found;
}

uint64_t
imdb_usage_map(imdb_db_t * const db,
               char     ** const map) {
  const int blk_size = 4096;
  imdb_block_t blk;
  uint64_t records;
  unsigned char *r; /* mnemonics : block, record */
  char *m = NULL;   /* mnemonics : map */

  memset(&blk, 0x0, sizeof(imdb_block_t));

  records = imdb_records_count(db);
  CALLOC(*map, records + 1, sizeof(char));

  m = *map;
  blk.start = 1;
  blk.records = blk_size;

  while (imdb_read_blk(db, &blk) > 0) {
    r = blk.data;
    for (unsigned int i = 0;  i < blk.records;  i++, m++, r += IMDB_REC_LEN) {
      *m = (r[REC_OFF_RU] == 0xFF) ? CHAR_USED : CHAR_NONE;
    }
    blk.start += blk_size;
  }
  FREE(blk.data);

  return records;
}

uint16_t
imdb_usage_slice(imdb_db_t * const db,
                 char     ** const map,
                 uint64_t  offset,
                 uint16_t  limit) {
  imdb_block_t blk;
  unsigned char *r; /* mnemonics : block, record */
  char *m = NULL;   /* mnemonics : map */

  memset(&blk, 0x0, sizeof(imdb_block_t));
  CALLOC(*map, limit + 1, sizeof(char));

  m = *map;
  blk.start = offset;
  blk.records = limit;

  limit = imdb_read_blk(db, &blk);
  r = blk.data;
  for (uint16_t i = 0;  i < blk.records;  i++, m++, r += IMDB_REC_LEN) {
    *m = (r[REC_OFF_RU] == 0xFF) ? CHAR_USED : CHAR_NONE;
  }

  return limit;
}
