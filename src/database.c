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

struct _simdb_t {
  int fd;
  int flags;
  char path[PATH_MAX];
};

#define DB_READ(db, buf, len, off) \
  errno = 0; \
  memset((buf), 0x0, (len)); \
  bytes = pread((db)->fd, (buf), (len), (off)); \
  if (errno) { return SIMDB_ERR_SYSTEM; }

#define DB_WRITE(db, buf, len, off) \
  errno = 0; \
  bytes = pwrite((db)->fd, (buf), (len), (off)); \
  if (errno) { return SIMDB_ERR_SYSTEM; }

const char *simdb_hdr_fmt = "IMDB v%02u, CAPS: %s;";

bool
simdb_create(const char *path) {
  ssize_t bytes = 0;
  unsigned char buf[SIMDB_REC_LEN];
  const char *caps = "M-R";
  bool result = false;
  int fd = -1;

  memset(buf, 0x0, sizeof(char) * SIMDB_REC_LEN);

  if ((fd = creat(path, 0644)) < 0)
    return result;

  snprintf((char *) buf, SIMDB_REC_LEN, simdb_hdr_fmt, SIMDB_VERSION, caps);

  bytes = pwrite(fd, buf, SIMDB_REC_LEN, 0);
  if (bytes == SIMDB_REC_LEN)
    result = true; /* success */

  close(fd);

  return result;
}

simdb_t *
simdb_open(const char *path, int mode, int *error) {
  simdb_t *db = NULL;
  ssize_t bytes = 0;
  struct stat st;
  char buf[SIMDB_REC_LEN] = "\0";
  int flags = 0, fd = -1;
  char *p;

  assert(path  != NULL);
  assert(error != NULL);

  if (stat(path, &st) < 0) {
    *error = SIMDB_ERR_SYSTEM;
    return NULL;
  }

  if ((fd = open(path, (mode & SIMDB_FLAG_WRITE) ? O_RDWR : O_RDONLY)) < 0) {
    *error = SIMDB_ERR_SYSTEM;
    return NULL;
  }

  errno = 0;
  bytes = pread(fd, buf, SIMDB_REC_LEN, 0);
  if (bytes < SIMDB_REC_LEN) {
    *error = errno ? SIMDB_ERR_SYSTEM : SIMDB_ERR_CORRUPTDB;
    return NULL;
  }

  p = buf + 0;
  if (memcmp("IMDB", p, 4) != 0) {
    *error = SIMDB_ERR_CORRUPTDB;
    return NULL;
  }

  p = buf + 6;
  if (atoi(p) != SIMDB_VERSION) {
    *error = SIMDB_ERR_WRONGVERS;
    return NULL;
  }

  p = buf + 10;
  if (memcmp("CAPS", p, 4) != 0) {
    *error = -3; /* Can't read database capabilities */
    return NULL;
  }

  if (mode & SIMDB_FLAG_WRITE)
    flags |= SIMDB_FLAG_WRITE;

  /* all seems to be ok */

  if ((db = calloc(1, sizeof(simdb_t))) == NULL) {
    *error = SIMDB_ERR_OOM;
    return NULL;
  }

  p = buf + 16;
  for (size_t i = 0; i < 8 && *p != '\0'; p++) {
    switch (*p) {
      case 'M' : flags |= SIMDB_CAP_BITMAP; break;
      case 'C' : flags |= SIMDB_CAP_COLORS; break;
      case 'R' : flags |= SIMDB_CAP_RATIO;  break;
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
simdb_close(simdb_t *db)
{
  assert(db != NULL);

  if (db->fd >= 0)
    close(db->fd);
}

const char *
simdb_error(int error) {
   if (error == SIMDB_SUCCESS) {
     return "success";
   } else if (error == SIMDB_ERR_SYSTEM) {
     return strerror(errno);
   } else if (error == SIMDB_ERR_OOM) {
     return "can't allocate memory";
   } else if (error == SIMDB_ERR_CORRUPTDB) {
     return "database corrupted";
   } else if (error == SIMDB_ERR_WRONGVERS) {
     return "database version differs from library version";
   } else if (error == SIMDB_ERR_READONLY) {
     return "database opened in read-only mode";
   } else if (error == SIMDB_ERR_NXRECORD) {
     return "no such record in database";
   }
   return "unknown error";
}

int simdb_read_rec(simdb_t *db, simdb_rec_t *rec)
{
  ssize_t bytes = 0;

  assert(db  != NULL);
  assert(rec != NULL);
  assert(rec->num > 0);

  DB_READ(db, rec->data, SIMDB_REC_LEN, rec->num * SIMDB_REC_LEN);

  if (bytes != SIMDB_REC_LEN)
    return errno ? SIMDB_ERR_SYSTEM : SIMDB_ERR_NXRECORD;

  if (rec->data[0] != 0xFF)
    return 0;

  return 1;
}

int simdb_write_rec(simdb_t *db, simdb_rec_t *rec)
{
  ssize_t bytes = 0;

  assert(db  != NULL);
  assert(rec != NULL);
  assert(rec->num > 0);

  if (!(db->flags & SIMDB_FLAG_WRITE))
    return SIMDB_ERR_READONLY;

  DB_WRITE(db, rec->data, SIMDB_REC_LEN, rec->num * SIMDB_REC_LEN);

  if (bytes != SIMDB_REC_LEN)
    return SIMDB_ERR_SYSTEM;

  return 1;
}

int simdb_read_blk(simdb_t *db, simdb_block_t *blk)
{
  ssize_t bytes = 0;

  assert(db  != NULL);
  assert(blk != NULL);
  assert(blk->start > 0);
  assert(blk->records > 0);

  FREE(blk->data);
  CALLOC(blk->data, blk->records, SIMDB_REC_LEN);
  DB_READ(db, blk->data, blk->records * SIMDB_REC_LEN, blk->start * SIMDB_REC_LEN);
  blk->records = bytes / SIMDB_REC_LEN;

  return blk->records;
}

uint64_t
simdb_records_count(simdb_t * const db) {
  struct stat st;
  off_t size = 0;

  memset(&st, 0x0, sizeof(struct stat));

  fstat(db->fd, &st);
  size = st.st_size;

  return size / SIMDB_REC_LEN;
}

static float
ratio_from_rec_data(unsigned char * const data) {
  uint16_t iw, ih;

  iw = *((uint16_t *)(data + REC_OFF_IW));
  ih = *((uint16_t *)(data + REC_OFF_IH));

   return (iw > 0 && ih > 0) ? ((float) iw / ih) : 0.0;
}

int
simdb_search(simdb_t        * const db,
             simdb_rec_t    * const sample,
             simdb_search_t * const search,
             simdb_match_t  **matches)
{
  simdb_block_t blk;
  simdb_match_t match;
  const int blk_size = 4096;
  uint64_t found = 0;
  unsigned int i = 0;
  unsigned char *p = NULL;
  float ratio_s = 0.0; /* source */
  float ratio_t = 0.0; /* tested */
  int ret = 0;

  assert(db      != NULL);
  assert(sample  != NULL);
  assert(search  != NULL);
  assert(matches != NULL);
  assert(search->maxdiff_ratio  >= 0.0 && search->maxdiff_ratio  <= 1.0);
  assert(search->maxdiff_bitmap >= 0.0 && search->maxdiff_bitmap <= 1.0);

  memset(&blk,   0x0, sizeof(simdb_block_t));
  memset(&match, 0x0, sizeof(simdb_match_t));
  blk.start = 1;
  blk.records = blk_size;

  if ((ret = simdb_read_rec(db, sample)) < 1)
    return ret;

  if (search->limit == 0)
    search->limit = -1; /* unsigned -> max */

  if (search->maxdiff_ratio > 0.0)
    ratio_s = ratio_from_rec_data(sample->data);

  CALLOC(*matches, search->limit, sizeof(simdb_match_t));

  while (simdb_read_blk(db, &blk) > 0) {
    p = blk.data;
    for (i = 0; i < blk.records; i++, p += SIMDB_REC_LEN) {
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
      memcpy(&(*matches)[found], &match, sizeof(simdb_match_t));
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
simdb_usage_map(simdb_t * const db,
               char    ** const map) {
  const int blk_size = 4096;
  simdb_block_t blk;
  uint64_t records;
  unsigned char *r; /* mnemonics : block, record */
  char *m = NULL;   /* mnemonics : map */

  memset(&blk, 0x0, sizeof(simdb_block_t));

  records = simdb_records_count(db);
  CALLOC(*map, records + 1, sizeof(char));

  m = *map;
  blk.start = 1;
  blk.records = blk_size;

  while (simdb_read_blk(db, &blk) > 0) {
    r = blk.data;
    for (unsigned int i = 0;  i < blk.records;  i++, m++, r += SIMDB_REC_LEN) {
      *m = (r[REC_OFF_RU] == 0xFF) ? CHAR_USED : CHAR_NONE;
    }
    blk.start += blk_size;
  }
  FREE(blk.data);

  return records;
}

uint16_t
simdb_usage_slice(simdb_t   * const db,
                  char     ** const map,
                  uint64_t  offset,
                  uint16_t  limit) {
  simdb_block_t blk;
  unsigned char *r; /* mnemonics : block, record */
  char *m = NULL;   /* mnemonics : map */

  memset(&blk, 0x0, sizeof(simdb_block_t));
  CALLOC(*map, limit + 1, sizeof(char));

  m = *map;
  blk.start = offset;
  blk.records = limit;

  limit = simdb_read_blk(db, &blk);
  r = blk.data;
  for (uint16_t i = 0;  i < blk.records;  i++, m++, r += SIMDB_REC_LEN) {
    *m = (r[REC_OFF_RU] == 0xFF) ? CHAR_USED : CHAR_NONE;
  }

  return limit;
}
