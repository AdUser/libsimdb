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
static const char *simdb_hdr_fmt = "IMDB v%02u, CAPS: %s;";

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

  if ((mode & SIMDB_FLAG_WRITE) && (mode & (SIMDB_FLAG_LOCK|SIMDB_FLAG_LOCKNB))) {
    int locktype = (mode & SIMDB_FLAG_LOCKNB) ? F_TLOCK : F_LOCK;
    if (lockf(fd, locktype, 0) < 0) {
      *error = SIMDB_ERR_LOCK;
      return NULL;
    }
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

  flags = mode & SIMDB_FLAGS_MASK;

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

  FREE(db);
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
  } else if (error == SIMDB_ERR_LOCK) {
    return "can't add lock on database file";
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

bool
simdb_record_used(simdb_t *db, int num) {
  simdb_urec_t *rec = NULL;
  bool ret = false;

  assert(db != NULL);

  if (num <= 0 || num > db->records)
    return false;

  if (simdb_read(db, num, 1, &rec) < 1)
    return false;

  ret = rec->used ? true : false;

  FREE(rec);
  return ret;
}

int
simdb_record_add(simdb_t *db, int num, const char *path, int flags) {
  simdb_urec_t *rec = NULL;

  assert(db != NULL);

  if (num < 0 || path == NULL)
    return SIMDB_ERR_USAGE;

  if (flags & SIMDB_ADD_NOEXTEND && num > db->records)
    return 0;

  if (access(path, R_OK) < 0)
    return SIMDB_ERR_SYSTEM;

  if (num > 0 && flags & SIMDB_ADD_NOREPLACE && simdb_record_used(db, num))
    return 0;

  if ((rec = simdb_record_create(path)) == NULL)
    return SIMDB_ERR_SAMPLER;

  if (num == 0)
    num = db->records + 1;

  num = simdb_write(db, num, 1, rec);

  FREE(rec);
  return num;
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
simdb_record_bitmap(simdb_t *db, int num, char **map, size_t *side) {
  simdb_urec_t *rec;
  int ret = 0;

  assert(db != NULL);

  if (num < 0 || map == NULL || side == NULL)
    return SIMDB_ERR_USAGE;

  if ((ret = simdb_read(db, num, 1, &rec)) <= 0)
    return ret;

  ret = simdb_bitmap_unpack(rec->bitmap, map);
  *side = SIMDB_BITMAP_SIDE;

  FREE(rec);
  return ret;
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

void
simdb_search_init(simdb_search_t *search) {
  assert(search != NULL);

  memset(search, 0x0, sizeof(simdb_search_t));

  search->d_ratio  = 0.07; /* 7% */
  search->d_bitmap = 0.07; /* 7% */

  return;
}

void
simdb_search_free(simdb_search_t *search) {
  assert(search != NULL);

  if (search->found == 0)
    return;
  FREE(search->matches);
  search->found = 0;
}

int
simdb_search(simdb_t *db, simdb_search_t *search, simdb_urec_t *sample) {
  simdb_match_t *matches;
  simdb_match_t match;
  simdb_urec_t *rec, *data = NULL;
  const int blksize = 4096;
  float ratio_s = 0.0; /* source */
  float ratio_t = 0.0; /* tested */
  int ret = 0, found = 0, capacity = 16;

  assert(db      != NULL);
  assert(search  != NULL);

  if (search->d_ratio  < 0.0 && search->d_ratio  > 1.0)
    return SIMDB_ERR_USAGE;
  if (search->d_bitmap < 0.0 && search->d_bitmap > 1.0)
    return SIMDB_ERR_USAGE;

  memset(&match, 0x0, sizeof(simdb_match_t));

  if (search->limit == 0)
    search->limit = INT_MAX;

  if (search->d_ratio > 0.0)
    ratio_s = simdb_record_ratio(sample);

  if (search->found)
    simdb_search_free(search);

  if ((matches = calloc(capacity, sizeof(simdb_match_t))) == NULL)
    return SIMDB_ERR_OOM;

  for (int num = 1; ; num += blksize) {
    ret = simdb_read(db, num, blksize, &data);
    if (ret == 0)
      break; /* end of records */
    if (ret < 0) {
      FREE(matches);
      return ret; /* error */
    }
    rec = data;
    for (int i = 0; i < ret; i++, rec++) {
      if (!rec->used)
        continue; /* record missing */

      match.d_ratio  = 0.0;
      match.d_bitmap = 0.0;

      /* - compare ratio - cheap */
      /* TODO: check caps */
      if (ratio_s > 0.0 && (ratio_t = simdb_record_ratio(rec)) > 0.0) {
        match.d_ratio  =  ratio_s - ratio_t;
        match.d_ratio *= (ratio_s > ratio_t) ? 1.0 : -1.0;
        if (match.d_ratio > search->d_ratio)
          continue;
      } else {
        /* either source or target ratio not set, can't compare, skip test */
      }
      /* - compare bitmap - more expensive */
      match.d_bitmap = simdb_bitmap_compare(rec->bitmap, sample->bitmap) / SIMDB_BITMAP_BITS;
      if (match.d_bitmap > search->d_bitmap)
        continue;
      /* whoa! a match found */
      /* allocate more memory for results array if needed */
      if (found == capacity) {
        simdb_match_t *tmp = NULL;
        capacity *= 2;
        if ((tmp = realloc(matches, capacity)) == NULL) {
          /* fuck! */
          FREE(matches);
          FREE(data);
          return SIMDB_ERR_OOM;
        }
        matches = tmp; /* successfully relocated */
      }
      /* copy match to results array */
      match.num = num + i;
      memcpy(&matches[found], &match, sizeof(simdb_match_t));
      found++;
      if (found >= search->limit)
        break;
    }
    FREE(data);
    if (found >= search->limit)
      break;
  }

  if (found) {
    search->found   = found;
    search->matches = matches;
  } else {
    FREE(matches);
  }

  return found;
}

int
simdb_search_byid(simdb_t *db, simdb_search_t *search, int num) {
  simdb_urec_t *sample;
  int ret = 0;

  assert(db     != NULL);
  assert(search != NULL);

  if (num <= 0)
    return SIMDB_ERR_USAGE;

  if ((ret = simdb_read(db, num, 1, &sample)) < 1)
    return ret;

  ret = simdb_search(db, search, sample);
  FREE(sample);

  return ret;
}

int
simdb_search_file(simdb_t *db, simdb_search_t *search, const char *path) {
  simdb_urec_t *sample = NULL;
  int ret = 0;

  assert(db     != NULL);
  assert(search != NULL);

  if (path == NULL)
    return SIMDB_ERR_USAGE;

  if ((sample = simdb_record_create(path)) == NULL)
    return SIMDB_ERR_SAMPLER;

  ret = simdb_search(db, search, sample);
  FREE(sample);

  return ret;
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
  if ((m = calloc(records + 1, sizeof(char))) == NULL)
    return SIMDB_ERR_OOM;
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
      *m = (r->used == 0xFF) ? 0x1 : 0x0;
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

  if ((m = calloc(limit + 1, sizeof(char))) == NULL)
    return SIMDB_ERR_OOM;
  m = *map;

  r = data;
  for (int i = 0; i < limit; i++, m++, r++) {
    *m = (r->used == 0xFF) ? 0x1 : 0x0;
  }
  FREE(data);

  return ret;
}
