#include "../src/common.h"
#include "../src/database.h"

int main()
{
  imdb_db_t *db;
  imdb_rec_t rec[2];
  imdb_block_t blk;
  char *path = "test.db";
  int mode = 0, err = 0, num;
  bool ret;

  unlink(path);

  db = imdb_open(path, mode, &err);
  assert(db == NULL);
  assert(err == -1); /* no such file */

  ret = imdb_create(path);
  assert(ret == true);

  db = imdb_open(path, mode, &err);
  assert(db != NULL);

  rec[0].num = 1;
  err = imdb_read_rec(db, rec);
  assert(err < 0); /* no such record */

  memset(rec[0].data, 0xAA, IMDB_REC_LEN);
  memset(rec[0].data, 0xFF, 1); /* record is used */

  num = imdb_write_rec(db, rec);
  assert(num != 1); /* database open in read-only mode */

  imdb_close(db);

  mode |= IMDB_FLAG_WRITE;
  db = imdb_open(path, mode, &err);
  assert(db != NULL);

  num = imdb_write_rec(db, rec);
  assert(num == 1); /* success */

  num = imdb_read_rec(db, rec);
  assert(num == 1);

  blk.start   = 1;
  blk.records = 2;
  blk.data    = NULL;
  num = imdb_read_blk(db, &blk);
  assert(num == 1);
  assert(blk.records == 1);
  assert(blk.data != NULL);

  rec[0].num = 1;
  rec[1].num = 3;

  imdb_close(db);

  return 0;
}
