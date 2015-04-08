#include "common.h"
#include "database.h"
#include "bitmap.h"
#include "sampler.h"

int main(int argc, char **argv)
{
  imdb_rec_t rec;

  if (argc < 2) {
    puts("Usage: sampler-test <file>");
    exit(EXIT_FAILURE);
  }

  memset(&rec, 0x0, sizeof(imdb_rec_t));

  if (imdb_sample(&rec, argv[1]) != 0) {
    puts("sampler failure");
    exit(EXIT_FAILURE);
  }

  bitmap_print(&rec.data[REC_OFF_BM]);

  exit(EXIT_SUCCESS);
}
