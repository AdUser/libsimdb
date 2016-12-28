#include "../src/common.h"
#include "../src/database.h"

int main() {
  simdb_t *db;
  simdb_rec_t rec[2];
  simdb_block_t blk;
  char *path = "test.db";
  int mode = 0, err = 0, num;
  bool ret;

  unlink(path);

  db = simdb_open(path, mode, &err);
  assert(db == NULL);
  assert(err == -1); /* no such file */

  ret = simdb_create(path);
  assert(ret == true);

  db = simdb_open(path, mode, &err);
  assert(db != NULL);

  rec[0].num = 1;
  err = simdb_read_rec(db, rec);
  assert(err == SIMDB_ERR_NXRECORD);

  memset(rec[0].data, 0xAA, SIMDB_REC_LEN);
  memset(rec[0].data, 0xFF, 1); /* record is used */

  num = simdb_write_rec(db, rec);
  assert(num == SIMDB_ERR_READONLY); /* database open in read-only mode */

  simdb_close(db);

  mode |= SIMDB_FLAG_WRITE;
  db = simdb_open(path, mode, &err);
  assert(db != NULL);

  num = simdb_write_rec(db, rec);
  assert(num == 1); /* success */

  num = simdb_read_rec(db, rec);
  assert(num == 1);

  blk.start   = 1;
  blk.records = 2;
  blk.data    = NULL;
  num = simdb_read_blk(db, &blk);
  assert(num == 1);
  assert(blk.records == 1);
  assert(blk.data != NULL);

  rec[0].num = 1;
  rec[1].num = 3;

  simdb_close(db);

  return 0;
}
