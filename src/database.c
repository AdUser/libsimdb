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

int imdb_init(imdb_t *db, const char *path)
{
  ssize_t bytes = 0;
  unsigned char buf[IMDB_REC_LEN];

  memset(buf, 0x0, sizeof(char) * IMDB_REC_LEN);

  if ((db->fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0644)) == -1) {
    db->errstr = strerror(errno);
    return -1;
  }

  snprintf((char *) buf, IMDB_REC_LEN, imdb_hdr_fmt, IMDB_VERSION, "M-R");
  DB_SEEK(db, 0);
  DB_WRITE(db, buf, IMDB_REC_LEN);

  if (bytes != IMDB_REC_LEN) {
    db->errstr = "Can't write database header";
    return -1;
  }

  return 0;
}

int imdb_open(imdb_t *db, const char *path)
{
  ssize_t bytes = 0;
  struct stat st;
  char buf[IMDB_REC_LEN] = "\0";

  assert(db   != NULL);
  assert(path != NULL);

  memset(db, 0x0, sizeof(imdb_t));
  db->fd = -1;

  errno = 0;
  if (stat(path, &st) == -1) {
    db->errstr = strerror(errno);
    return -1;
  }

  errno = 0;
  if ((db->fd = open(path, OPEN_FLAGS)) == -1) {
    db->errstr = strerror(errno);
    return -1;
  }
  db->path = path;

  memset(buf, 0x0, IMDB_REC_LEN);
  DB_SEEK(db, 0);
  DB_READ(db, buf, IMDB_REC_LEN);
  if (bytes != IMDB_REC_LEN) {
    db->errstr = "Empty or damaged database file";
    return -1;
  }
  if (memcmp("IMDB", buf, 4) != 0) {
    db->errstr = "Not a database file";
    return -1;
  }
  if (atoi(buf + 6) != IMDB_VERSION) {
    db->errstr = "Database version mismatch";
    return -1;
  }
  if (memcmp("CAPS", buf + 10, 4) != 0) {
    db->errstr = "Can't read database capabilities";
    return -1;
  }
  memcpy(db->caps, buf + 16, sizeof(char) * 8);

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

  if (bytes != IMDB_REC_LEN)
    return -1;

  if (rec->data[0] != 0xFF)
    return 0;

  return 1;
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

int imdb_read_blk(imdb_t *db, imdb_block_t *blk)
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

uint64_t
imdb_records_count(imdb_t * const db) {
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

  return (iw > 0 && ih > 0) ? (iw / ih) : 0.0;
}

int
imdb_search(imdb_t        * const db,
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
    db->errstr = "Can't read source sample";
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
imdb_usage_map(imdb_t * const db,
               char  ** const map) {
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
