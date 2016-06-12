#include "../src/common.h"
#include "../src/database.h"

int main()
{
  imdb_db_t db;
  imdb_rec_t rec[2];
  imdb_block_t blk;
  char *path = "test.db";

  unlink(path);
  assert(imdb_open(&db, path, 1) == 1);

  rec[0].num = 1;
  assert(imdb_read_rec(&db, rec) == 0);

  memset(rec[0].data, 0xAA, IMDB_REC_LEN);
  assert(imdb_write_rec(&db, rec) == 1);

  assert(imdb_read_rec(&db, rec) == 1);

  blk.start   = 1;
  blk.records = 2;
  blk.data    = NULL;
  assert(imdb_read_blk(&db, &blk) == 1);
  assert(blk.records == 1);
  assert(blk.data != NULL);

  rec[0].num = 1;
  rec[1].num = 3;

  assert(imdb_close(&db) == 0);

  return 0;
}
