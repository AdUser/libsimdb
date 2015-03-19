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

#define DB_SEEK(db, offset) \
  errno = 0; \
  if (lseek((db)->fd, (offset), SEEK_SET) == -1) { \
    (db)->errstr = strerror(errno); \
    return -1; \
  }

#define DB_READ(db, buf, len) \
  errno = 0; \
  memset((buf), 0x0, (len)); \
  bytes = read((db)->fd, (buf), (len)); \
  if (errno) { \
    (db)->errstr = strerror(errno); \
    return -1; \
  }

#define DB_WRITE(db, buf, len) \
  errno = 0; \
  bytes = write((db)->fd, (buf), (len)); \
  if (errno) { \
    (db)->errstr = strerror(errno); \
    return -1; \
  }

const char *imdb_hdr_fmt = "IMDB v%02u, CAPS: %s;";

int imdb_open(imdb_t *db, const char *path)
{
  int init = 0;
  ssize_t bytes = 0;
  struct stat st;
  char buf[IMDB_REC_LEN] = "\0";

  assert(db   != NULL);
  assert(path != NULL);

  memset(db, 0x0, sizeof(imdb_t));
  db->fd = -1;

  errno = 0;
  if (stat(path, &st) == -1 && errno == ENOENT) {
    init = 1;
  }

  errno = 0;
  if ((db->fd = open(path, OPEN_FLAGS, 0644)) == -1) {
    db->errstr = strerror(errno);
    return -1;
  }

  db->path = path;

  if (init) {
    memset(buf, 0x0, IMDB_REC_LEN);
    snprintf((char *) buf, IMDB_REC_LEN, imdb_hdr_fmt, IMDB_VERSION, "M-R");
    DB_SEEK(db, 0);
    DB_WRITE(db, buf, IMDB_REC_LEN);

    return bytes / IMDB_REC_LEN;
  }

  return 0;
}

int imdb_close(imdb_t *db)
{
  assert(db != NULL);

  errno = 0;
  if (close(db->fd) != 0) {
    db->errstr = strerror(errno);
    return -1;
  }
  db->fd = -1;

  return 0;
}

int imdb_read_rec(imdb_t *db, imdb_rec_t *rec)
{
  ssize_t bytes = 0;

  assert(db  != NULL);
  assert(rec != NULL);
  assert(rec->num > 0);

  DB_SEEK(db, rec->num * IMDB_REC_LEN);
  DB_READ(db, rec->data, IMDB_REC_LEN);

  return bytes / IMDB_REC_LEN;
}

int imdb_write_rec(imdb_t *db, imdb_rec_t *rec)
{
  ssize_t bytes = 0;

  assert(db  != NULL);
  assert(rec != NULL);
  assert(rec->num > 0);

  DB_SEEK(db, rec->num * IMDB_REC_LEN);
  DB_WRITE(db, rec->data, IMDB_REC_LEN);

  return bytes / IMDB_REC_LEN;
}

int imdb_read_blk(imdb_t *db, block_t *blk)
{
  ssize_t bytes = 0;

  assert(db  != NULL);
  assert(blk != NULL);
  assert(blk->start > 0);
  assert(blk->records > 0);

  FREE(blk->data);
  CALLOC(blk->data, blk->records, IMDB_REC_LEN);
  DB_SEEK(db, blk->start * IMDB_REC_LEN);
  DB_READ(db, blk->data, blk->records * IMDB_REC_LEN);
  blk->records = bytes / IMDB_REC_LEN;

  return blk->records;
}

int imdb_read_list(imdb_t *db, imdb_rec_t *list, size_t list_len)
{
  imdb_rec_t *r = NULL;
  ssize_t bytes;
  unsigned int i = 0;
  unsigned int processed = 0;

  assert(db   != NULL);
  assert(list != NULL);
  assert(list_len > 0);

  r = list;
  for (i = 0; i < list_len; i++, r++) {
    DB_SEEK(db, r->num * IMDB_REC_LEN);
    DB_READ(db, r->data, IMDB_REC_LEN);
    if (bytes == IMDB_REC_LEN) { processed++; }
  }

  return processed;
}

int imdb_write_list(imdb_t *db, imdb_rec_t *list, size_t list_len)
{
  imdb_rec_t *r = NULL;
  ssize_t bytes;
  unsigned int i = 0;
  unsigned int processed = 0;

  assert(db   != NULL);
  assert(list != NULL);
  assert(list_len > 0);

  r = list;
  for (i = 0; i < list_len; i++, r++) {
    DB_SEEK(db, r->num * IMDB_REC_LEN);
    DB_WRITE(db, r->data, IMDB_REC_LEN);
    if (bytes == IMDB_REC_LEN) { processed++; }
  }

  return processed;
}

int imdb_search(imdb_t *db, imdb_rec_t *sample, float tresh, match_t **matches)
{
  const int blk_size = 4096;
  uint64_t found = 0;
  block_t blk;
  unsigned int i = 0;
  unsigned char *p = NULL, *t = NULL;
  float diff = 0.0;

  assert(db      != NULL);
  assert(sample  != NULL);
  assert(matches != NULL);

  memset(&blk, 0x0, sizeof(block_t));
  blk.start = 1;
  blk.records = blk_size;

  if (imdb_read_rec(db, sample) < 1) {
    db->errstr = "Can't read source sample";
    return -1;
  }

  if (!sample->data[0]) {
    db->errstr = "Source sample not exists";
    return -1;
  }

  *matches = NULL;
  while (imdb_read_blk(db, &blk) > 0) {
    p = blk.data;
    for (i = 0; i < blk.records; i++, p += IMDB_REC_LEN) {
      t = p + REC_OFF_RU;
      if (*t == 0x0) continue;

      t = p + REC_OFF_BM;
      diff  = (float) bitmap_compare(t, sample->data + REC_OFF_BM);
      diff /= BITMAP_BITS;
      if (diff > tresh) continue;

      /* allocate more memory, if needed */
      if (found % 10 == 0) {
        *matches = realloc(*matches, (found + 10) * sizeof(match_t));
        if (*matches == NULL) {
          db->errstr = "Memory allocation error";
          FREE(blk.data);
          return -1;
        }
        memset(&(*matches)[found], 0x0, sizeof(match_t) * 10);
      }

      /* create match record */
      (*matches)[found].num  = blk.start + i;
      (*matches)[found].diff = diff;
      found++;
    }
    FREE(blk.data);
    blk.start += blk_size;
  }

  return found;
}
