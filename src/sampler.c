#include "common.h"
#include "database.h"
#include "bitmap.h"
#include "sample.h"

void usage() {
  puts("Usage: imdb-sample <db path> <id> <sample path>");
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
  imdb_db_t  db;
  imdb_rec_t rec;

  memset(&db,  0x0, sizeof(imdb_db_t));
  memset(&rec, 0x0, sizeof(imdb_rec_t));

  if (argc < 4)
    usage();

  rec.num = strtoll(argv[2], NULL, 10);
  if (errno != 0) {
    puts("arg 1 not a number");
    usage();
  }

  if (imdb_open(&db, argv[1]) != 0) {
    puts(db.errstr);
    usage();
  }

  if (imdb_sample(&rec, argv[3]) != 0) {
    puts("sampler failure");
    usage();
  }

  if(imdb_write_rec(&db, &rec) < 1) {
    puts(db.errstr);
    usage();
  }

  imdb_close(&db);
  bitmap_print(&rec.data[REC_OFF_BM]);

  exit(EXIT_SUCCESS);
}
