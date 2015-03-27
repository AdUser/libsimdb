#include "common.h"
#include "database.h"
#include "sampler.h"

int main(int argc, char **argv)
{
  char * const source = "sample.png";
  imdb_rec_t rec;
  int ret = 0;

  memset(&rec, 0x0, sizeof(imdb_rec_t));
  ret = imdb_sample(&rec, source);

  exit(ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
