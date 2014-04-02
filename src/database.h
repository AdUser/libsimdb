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
  uint64_t num;
  unsigned char data[REC_LEN];
} rec_t;

extern int db_open(db_t *db, const char *path);
extern int db_close(db_t *db);

extern int db_rd_rec(db_t *db, rec_t *rec);
extern int db_wr_rec(db_t *db, rec_t *rec);

extern int db_rd_blk(db_t *db, uint64_t start, size_t len, rec_t *list);
extern int db_wr_blk(db_t *db, uint64_t start, size_t len, rec_t *list);

extern int db_rd_list(db_t *db, rec_t *list, size_t list_len);
extern int db_wr_list(db_t *db, rec_t *list, size_t list_len);

#endif
#define HAS_DATABASE_H 1
