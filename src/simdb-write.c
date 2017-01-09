#include "common.h"
#include "bitmap.h"
#include "record.h"
#include "io.h"
#include "simdb.h"

#include <getopt.h>

void usage(int exitcode) {
  fprintf(stderr,
"Usage: simdb-write <opts>\n"
"  -b <path>   Path to database\n"
);
  fprintf(stderr,
"  -A <num>,<path>  Add sample from 'path' as record 'num'\n"
"  -D <num>         Delete record <num>\n"
"  -I               Create database (init)\n"
);
  exit(exitcode);
}

int main(int argc, char **argv)
{
  enum { undef, add, del, init } mode = undef;
  char opt;
  const char *db_path = NULL;
  const char *sample = NULL;
  const char *c = NULL;
  simdb_t  *db = NULL;
  simdb_urec_t rec;
  int num = 0, err = 0;

  memset(&rec, 0x0, sizeof(simdb_urec_t));

  if (argc < 3)
    usage(EXIT_FAILURE);

  while ((opt = getopt(argc, argv, "b:A:D:I")) != -1) {
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
        num = atoll(optarg);
        sample  = c + 1;
        break;
      case 'D' :
        mode = del;
        num = atoll(optarg);
        break;
      case 'I' :
        mode = init;
        break;
      default :
        usage(EXIT_FAILURE);
        break;
    }
  }

  if (db_path == NULL) {
    fprintf(stderr, "error: db path not set\n");
    usage(EXIT_FAILURE);
  }

  if (mode == init) {
    if (!simdb_create(db_path)) {
      fprintf(stderr, "database init: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
  }
  if ((db = simdb_open(db_path, 1, &err)) == NULL) {
    fprintf(stderr, "can't open database: %d\n", err);
    exit(EXIT_FAILURE);
  }

  switch (mode) {
    case add :
      if (num == 0 || sample == NULL)
        usage(EXIT_FAILURE);
      if ((err = simdb_record_add(db, num, sample, 0)) < 0) {
        fprintf(stderr, "%s\n", simdb_error(err));
        exit(EXIT_FAILURE);
      } else {
        fprintf(stderr, "added as record #%d", err);
      }
      break;
    case del :
      if ((err = simdb_record_del(db, num)) < 0) {
        fprintf(stderr, "%s\n", simdb_error(err));
        exit(EXIT_FAILURE);
      }
      break;
    case init :
      /* this case already handled above */
      break;
    default:
      usage(EXIT_FAILURE);
      break;
  }
  simdb_close(db);

  exit(EXIT_SUCCESS);
}
