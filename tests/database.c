#include "../src/common.h"
#include "../src/simdb.h"

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
  err = simdb_record_read(db, rec);
  assert(err == SIMDB_ERR_NXRECORD);

  memset(rec[0].data, 0xAA, SIMDB_REC_LEN);
  memset(rec[0].data, 0xFF, 1); /* record is used */

  num = simdb_record_write(db, rec);
  assert(num == SIMDB_ERR_READONLY); /* database open in read-only mode */

  simdb_close(db);

  mode |= SIMDB_FLAG_WRITE;
  db = simdb_open(path, mode, &err);
  assert(db != NULL);

  num = simdb_record_write(db, rec);
  assert(num == 1); /* success */

  num = simdb_record_read(db, rec);
  assert(num == 1);

  blk.start   = 1;
  blk.records = 2;
  blk.data    = NULL;
  num = simdb_block_read(db, &blk);
  assert(num == 1);
  assert(blk.records == 1);
  assert(blk.data != NULL);

  rec[0].num = 1;
  rec[1].num = 3;

  simdb_close(db);

  unlink(path);

  return 0;
}
