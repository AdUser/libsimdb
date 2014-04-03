#ifndef HAS_DATABASE_H

#define REC_LEN 48
#define DB_VERSION 1
#define OPEN_FLAGS O_CREAT | O_RDWR

typedef struct {
  int fd;
  const char *path;
  const char *errstr;
} db_t;

typedef struct {
  uint64_t start;
  size_t records;
  unsigned char *data;
} block_t;

/**
  pos len || description
 - 0   1  -- record is used
 - 1   1  -- must be zero
 - 2  32  -- bitmap, each 2 bytes is row of monochrome image 16x16
 - 33  1  -- level of color R__
 - 34  1  -- level of color _G_
 - 35  1  -- level of color __B
 - 36  11 -- must be zero
 */

#define OFF_USED   0
#define OFF_BITMAP 2
#define OFF_COLORR 33
#define OFF_COLORG 34
#define OFF_COLORB 35

typedef struct {
  uint64_t num;
  unsigned char data[REC_LEN];
} rec_t;

typedef struct {
  uint64_t num;
  float diff;
} match_t;

extern int db_open(db_t *db, const char *path);
extern int db_close(db_t *db);

extern int db_rd_rec(db_t *db, rec_t *rec);
extern int db_wr_rec(db_t *db, rec_t *rec);

extern int db_rd_blk(db_t *db, block_t *blk);
extern int db_wr_blk(db_t *db, block_t *blk);

extern int db_rd_list(db_t *db, rec_t *list, size_t list_len);
extern int db_wr_list(db_t *db, rec_t *list, size_t list_len);

#endif
#define HAS_DATABASE_H 1
