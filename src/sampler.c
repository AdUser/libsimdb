#include "common.h"
#include "database.h"
#include "bitmap.h"
#include "sample.h"

#include <getopt.h>

void usage(int exitcode) {
  puts(
"Usage: imdb-write <opts>\n"
"  -b <path>   Path to database\n"
);
puts(
"  -A <num>,<path>  Add sample from 'path' as record 'num'\n"
"  -D <num>         Delete record <num>\n"
);
  exit(exitcode);
}

int main(int argc, char **argv)
{
  enum { undef, add, del } mode = undef;
  char opt;
  const char *db_path = NULL;
  const char *sample = NULL;
  const char *c = NULL;
  imdb_db_t  db;
  imdb_rec_t rec;

  memset(&db,  0x0, sizeof(imdb_db_t));
  memset(&rec, 0x0, sizeof(imdb_rec_t));

  if (argc < 3)
    usage(EXIT_FAILURE);

  while ((opt = getopt(argc, argv, "b:A:D:")) != -1) {
    switch (opt) {
      case 'b' :
        db_path = optarg;
        break;
      case 'h' :
        usage(EXIT_SUCCESS);
        break;
      case 'A' :
        mode = add;
        if ((c = strchr(optarg, ',')) == NULL)
          usage(EXIT_FAILURE);
        rec.num = atoll(optarg);
        sample  = c + 1;
        break;
      case 'D' :
        mode = del;
        rec.num = atoll(optarg);
        break;
      default :
        usage(EXIT_FAILURE);
        break;
    }
  }

  if (db_path == NULL)
    usage(EXIT_FAILURE);

  if (imdb_open(&db, db_path, 1) != 0) {
    puts(db.errstr);
    exit(EXIT_FAILURE);
  }

  switch (mode) {
    case add :
      if (rec.num == 0 || sample == NULL)
        usage(EXIT_FAILURE);
      if (imdb_sample(&rec, sample) != 0) {
        puts("sampler failure");
        exit(EXIT_FAILURE);
      }
      if (imdb_write_rec(&db, &rec) < 1) {
        puts(db.errstr);
        exit(EXIT_FAILURE);
      }
      bitmap_print(&rec.data[REC_OFF_BM]);
      break;
    case del :
      if (imdb_write_rec(&db, &rec) < 1) {
        puts(db.errstr);
        exit(EXIT_FAILURE);
      }
      break;
    default:
      usage(EXIT_FAILURE);
      break;
  }
  imdb_close(&db);

  exit(EXIT_SUCCESS);
}
