#include "../src/main.h"
#include "../src/database.h"

int main(int argc, char **argv)
{
  db_t db;
  rec_t rec[2];
  char *path = "test.db";

  unlink(path);
  assert(db_open(&db, path) == 1);

  rec[0].num = 1;
  assert(db_rd_rec(&db, rec) == 0);

  memset(rec[0].data, 0xAA, REC_LEN);
  assert(db_wr_rec(&db, rec) == 1);

  assert(db_rd_rec(&db, rec) == 1);

  assert(db_rd_blk(&db, 1, 1, rec) == 1);
  assert(db_rd_blk(&db, 1, 2, rec) == 1);

  rec[0].num = 1;
  rec[1].num = 3;
  assert(db_rd_list(&db, rec, 2) == 1);

  assert(db_close(&db) == 0);

  return 0;
}
