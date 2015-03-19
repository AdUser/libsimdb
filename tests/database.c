#include "../src/common.h"
#include "../src/database.h"

int main(int argc, char **argv)
{
  imdb_t db;
  imdb_rec_t rec[2];
  block_t blk;
  char *path = "test.db";

  unlink(path);
  assert(db_open(&db, path) == 1);

  rec[0].num = 1;
  assert(db_rd_rec(&db, rec) == 0);

  memset(rec[0].data, 0xAA, IMDB_REC_LEN);
  assert(db_wr_rec(&db, rec) == 1);

  assert(db_rd_rec(&db, rec) == 1);

  blk.start   = 1;
  blk.records = 2;
  blk.data    = NULL;
  assert(db_rd_blk(&db, &blk) == 1);
  assert(blk.records == 1);
  assert(blk.data != NULL);

  rec[0].num = 1;
  rec[1].num = 3;
  assert(db_rd_list(&db, rec, 2) == 1);

  assert(db_close(&db) == 0);

  return 0;
}
