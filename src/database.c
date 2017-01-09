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

/**
 * @file
 * Common routines to work with database
 */

#include "common.h"
#include "bitmap.h"
#include "record.h"
#include "io.h"
#include "simdb.h"

struct _simdb_t {
  int fd;               /**< database file descriptor */
  int flags;            /**< database flags and capabilities, see SIMDB_FLAGS_* and SIMDB_CAP_* defines */
  int records;          /**< database records count */
  char path[PATH_MAX];  /**< path to database file */
};

/** database header format line */
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
  db->records = (st.st_size / SIMDB_REC_LEN) - 1;

  strncpy(db->path, path, sizeof(db->path));

  return db;
}

void
simdb_close(simdb_t *db) {
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
  } else if (error == SIMDB_ERR_USAGE) {
    return "wrong parameters passed to finction";
  } else if (error == SIMDB_ERR_SAMPLER) {
    return "given file not an image, damaged or has unsupported format";
  }
  return "unknown error";
}

int
simdb_read(simdb_t *db, int start, int records, simdb_urec_t **data) {
  simdb_urec_t *tmp;
  off_t offset = 0;
  ssize_t bytes = 0;

  assert(db != NULL);
  assert(data != NULL);

  if (start < 1 || records < 1)
    return SIMDB_ERR_USAGE;

  offset = SIMDB_REC_LEN * start;
  bytes  = SIMDB_REC_LEN * records;

  if ((tmp = calloc(1, bytes)) == NULL)
    return SIMDB_ERR_OOM;

  if ((bytes = pread(db->fd, tmp, bytes, offset)) < 0) {
    free(tmp);
    return SIMDB_ERR_SYSTEM;
  }

  records = bytes / SIMDB_REC_LEN;
  if (records <= 0) {
    free(tmp);
    return 0;
  }

  *data = tmp;
  return records;
}

int
simdb_write(simdb_t *db, int start, int records, simdb_urec_t *data) {
  off_t offset = 0;
  ssize_t bytes = 0;

  assert(db != NULL);
  assert(data != NULL);

  if (start < 1 || records < 1)
    return SIMDB_ERR_USAGE;

  if (!(db->flags & SIMDB_FLAG_WRITE))
    return SIMDB_ERR_READONLY;

  offset = SIMDB_REC_LEN * start;
  bytes  = SIMDB_REC_LEN * records;

  if ((bytes = pwrite(db->fd, data, bytes, offset)) < 0)
    return SIMDB_ERR_SYSTEM;

  records = bytes / SIMDB_REC_LEN;
  if (records <= 0)
    return 0;

  if ((start + records - 1) > db->records)
    db->records = (start + records - 1);

  return records;
}

int
simdb_record_del(simdb_t *db, int num) {
  simdb_urec_t *rec;
  int ret = 0;

  assert(db != NULL);

  if (num < 1)
    return SIMDB_ERR_USAGE;

  if (!(db->flags & SIMDB_FLAG_WRITE))
    return SIMDB_ERR_READONLY;

  if ((ret = simdb_read(db, num, 1, &rec)) < 1)
    return ret;

  rec->used = 0x0;

  if ((ret = simdb_write(db, num, 1, rec)) < 1)
    num = ret;

  FREE(rec);

  return num;
}

int
simdb_records_count(simdb_t * const db) {
  assert(db != NULL);
  return db->records;
}

inline static float
simdb_record_ratio(simdb_urec_t *r) {
  assert(r != NULL);

  if (r->image_w > 0 && r->image_h > 0)
    return (float) r->image_w / r->image_h;

  return 0.0;
}

int
simdb_search(simdb_t * const db, int num,
             simdb_search_t * const search,
             simdb_match_t  **matches)
{
  simdb_match_t match;
  simdb_urec_t *data = NULL;
  simdb_urec_t *rec, sample;
  const int blksize = 4096;
  uint64_t found = 0;
  float ratio_s = 0.0; /* source */
  float ratio_t = 0.0; /* tested */
  int ret = 0;

  assert(db      != NULL);
  assert(search  != NULL);
  assert(matches != NULL);
  assert(search->maxdiff_ratio  >= 0.0 && search->maxdiff_ratio  <= 1.0);
  assert(search->maxdiff_bitmap >= 0.0 && search->maxdiff_bitmap <= 1.0);

  memset(&match, 0x0, sizeof(simdb_match_t));

  if ((ret = simdb_read(db, num, 1, &rec)) < 1)
    return ret;

  memcpy(&sample, rec, sizeof(sample));
  FREE(rec);

  if (search->limit == 0)
    search->limit = -1; /* unsigned -> max */

  if (search->maxdiff_ratio > 0.0)
    ratio_s = simdb_record_ratio(&sample);

  CALLOC(*matches, search->limit, sizeof(simdb_match_t));

  for (num = 1; ; num += blksize) {
    ret = simdb_read(db, num, blksize, &data);
    if (ret < 0)
      return ret;
    rec = data;
    for (int i = 0; i < ret; i++, rec++) {
      if (!rec->used)
        continue; /* record missing */

      match.diff_ratio  = 0.0;
      match.diff_bitmap = 0.0;

      /* - compare ratio - cheap */
      /* TODO: check caps */
      if (ratio_s > 0.0 && (ratio_t = simdb_record_ratio(rec)) > 0.0) {
        match.diff_ratio  =  ratio_s - ratio_t;
        match.diff_ratio *= (ratio_s > ratio_t) ? 1.0 : -1.0;
        if (match.diff_ratio > search->maxdiff_ratio)
          continue;
      } else {
        /* either source or target ratio not set, can't compare, skip test */
      }

      /* - compare bitmap - more expensive */
      match.diff_bitmap = simdb_bitmap_compare(rec->bitmap, sample.bitmap) / SIMDB_BITMAP_BITS;
      if (match.diff_bitmap > search->maxdiff_bitmap)
        continue;

      /* create match record */
      match.num = num + i;
      memcpy(&(*matches)[found], &match, sizeof(simdb_match_t));
      found++;
      if (found >= search->limit)
        break;
    }
    FREE(data);
    if (found >= search->limit)
      break;
  }

  return found;
}

int
simdb_usage_map(simdb_t * const db, char ** const map) {
  const int blksize = 4096;
  simdb_urec_t *data = NULL;
  simdb_urec_t *r;  /* mnemonics : record pointer */
  char *m = NULL;   /* mnemonics : map pointer */
  int ret = 0, records = 0;

  assert(db  != NULL);
  assert(map != NULL);

  records = simdb_records_count(db);
  CALLOC(m, records + 1, sizeof(char));
  *map = m;

  for (int num = 1; ; num += blksize) {
    ret = simdb_read(db, num, blksize, &data);
    if (ret < 0) {
      FREE(*map);
      return ret;
    }
    if (ret == 0)
      break;
    r = data;
    for (int i = 0; i < ret; i++, m++, r++) {
      *m = (r->used == 0xFF) ? CHAR_USED : CHAR_NONE;
    }
    FREE(data);
  }

  return records;
}

int
simdb_usage_slice(simdb_t * const db, char ** const map, int offset, int limit) {
  simdb_urec_t *data = NULL;
  simdb_urec_t *r;  /* mnemonics : record pointer */
  char *m = NULL;   /* mnemonics : map pointer */
  int ret = 0;

  assert(db  != NULL);
  assert(map != NULL);

  if (offset < 1 || limit < 1)
    return SIMDB_ERR_USAGE;

  ret = simdb_read(db, offset, limit, &data);
  if (ret <= 0)
    return ret;

  CALLOC(m, limit + 1, sizeof(char));
  m = *map;

  r = data;
  for (int i = 0; i < limit; i++, m++, r++) {
    *m = (r->used == 0xFF) ? CHAR_USED : CHAR_NONE;
  }
  FREE(data);

  return ret;
}
