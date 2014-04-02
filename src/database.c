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

int db_open(db_t *db, const char *path)
{
  int init = 0;
  ssize_t bytes = 0;
  struct stat st;
  char buf[REC_LEN] = "\0";

  assert(db   != NULL);
  assert(path != NULL);

  memset(db, 0x0, sizeof(db_t));
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
    memset(buf, 0x0, REC_LEN);
    snprintf((char *) buf, REC_LEN, "DB of image fingerprints (vers %d)", DB_VERSION);
    DB_SEEK(db, 0);
    DB_WRITE(db, buf, REC_LEN);

    return bytes / REC_LEN;
  }

  return 0;
}

int db_close(db_t *db)
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

int db_rd_rec(db_t *db, rec_t *rec)
{
  ssize_t bytes = 0;

  assert(db  != NULL);
  assert(rec != NULL);
  assert(rec->num > 0);

  DB_SEEK(db, rec->num * REC_LEN);
  DB_READ(db, rec->data, REC_LEN);

  return bytes / REC_LEN;
}

int db_wr_rec(db_t *db, rec_t *rec)
{
  ssize_t bytes = 0;

  assert(db  != NULL);
  assert(rec != NULL);
  assert(rec->num > 0);

  DB_SEEK(db, rec->num * REC_LEN);
  DB_WRITE(db, rec->data, REC_LEN);

  return bytes / REC_LEN;
}

int db_rd_blk(db_t *db, block_t *blk)
{ 
  ssize_t bytes = 0;

  assert(db  != NULL);
  assert(blk != NULL);
  assert(blk->start > 0);
  assert(blk->records > 0);

  FREE(blk->data);
  CALLOC(blk->data, blk->records, REC_LEN);
  DB_SEEK(db, blk->start * REC_LEN);
  DB_READ(db, blk->data, blk->records * REC_LEN);
  blk->records = bytes / REC_LEN;

  return blk->records;
}

int db_rd_list(db_t *db, rec_t *list, size_t list_len)
{
  rec_t *r = NULL;
  ssize_t bytes;
  unsigned int i = 0;
  unsigned int processed = 0;

  assert(db   != NULL);
  assert(list != NULL);
  assert(list_len > 0);

  r = list;
  for (i = 0; i < list_len; i++, r++) {
    DB_SEEK(db, r->num * REC_LEN);
    DB_READ(db, r->data, REC_LEN);
    if (bytes == REC_LEN) { processed++; }
  }

  return processed;
}

int db_wr_list(db_t *db, rec_t *list, size_t list_len)
{
  rec_t *r = NULL;
  ssize_t bytes;
  unsigned int i = 0;
  unsigned int processed = 0;

  assert(db   != NULL);
  assert(list != NULL);
  assert(list_len > 0);

  r = list;
  for (i = 0; i < list_len; i++, r++) {
    DB_SEEK(db, r->num * REC_LEN);
    DB_WRITE(db, r->data, REC_LEN);
    if (bytes == REC_LEN) { processed++; }
  }

  return processed;
}
