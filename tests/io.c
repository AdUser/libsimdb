#include "../src/common.h"
#include "../src/record.h"
#include "../src/io.h"
#include "../src/simdb.h"

int main() {
  simdb_t *db;
  simdb_urec_t *data;
  simdb_urec_t rec[2];
  char *path = "test.db";
  int mode = 0, ret = 0;

  unlink(path);

  db = simdb_open(path, mode, &ret);
  assert(db == NULL);
  assert(ret == -1); /* no such file */

  ret = simdb_create(path);
  assert(ret == true);

  db = simdb_open(path, mode, &ret);
  assert(db != NULL);

  ret = simdb_read(db, 1, 1, &data);
  assert(ret == 0);

  memset(&rec[0], 0xAA, SIMDB_REC_LEN);
  memset(&rec[1], 0xAA, SIMDB_REC_LEN);
  rec[0].used = 1;
  rec[1].used = 1;

  ret = simdb_write(db, 1, 2, rec);
  assert(ret == SIMDB_ERR_READONLY); /* database open in read-only mode */

  simdb_close(db);

  mode |= SIMDB_FLAG_WRITE;
  db = simdb_open(path, mode, &ret);
  assert(db != NULL);

  ret = simdb_write(db, 1, 2, rec);
  assert(ret == 2); /* success */

  ret = simdb_read(db, 1, 0, &data);
  assert(ret == SIMDB_ERR_USAGE);

  ret = simdb_read(db, 0, 0, &data);
  assert(ret == SIMDB_ERR_USAGE);

  ret = simdb_read(db, 0, 1, &data);
  assert(ret == SIMDB_ERR_USAGE);

  /* valid request : single record */
  ret = simdb_read(db, 1, 1, &data);
  assert(ret == 1);
  free(data);

  /* valid request : multiple records */
  ret = simdb_read(db, 1, 2, &data);
  assert(ret == 2);
  free(data);

  /* range beyond end of file */
  ret = simdb_read(db, 1, 4, &data);
  assert(ret == 2);
  free(data);

  /* start beyond end of file */
  ret = simdb_read(db, 3, 4, &data);
  assert(ret == 0);

  simdb_close(db);

  unlink(path);

  return 0;
}
