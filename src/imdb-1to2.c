#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define IMDB_REC_LEN 48
#define BLK_SIZE 1000

void usage(const char *message) {
  if (message)
    printf("error: %s\n", message);
  printf("Usage: imdb_convert_1to2 <infile> <outfile>\n");
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
  int in, out;
  long unsigned int imdb_rec_total, rec_first, rec_last, records;
  unsigned char  in_buf[IMDB_REC_LEN * BLK_SIZE];
  unsigned char out_buf[IMDB_REC_LEN * BLK_SIZE];
  unsigned char  header[IMDB_REC_LEN];
  unsigned char *src, *dst;
  struct stat st;
  ssize_t bytes;

  memset(header, 0x0, IMDB_REC_LEN);
  snprintf((char *) header, IMDB_REC_LEN, "IMDB v%02u, CAPS: %s;", 2, "M-R");

  if (argc < 2)
    usage(NULL);

  errno = 0;
  if ((in  = open(argv[1], O_RDONLY)) < 0)
    usage(strerror(errno));

  if ((out = open(argv[2], O_WRONLY | O_TRUNC | O_CREAT, 0644)) < 0)
    usage(strerror(errno));

  if (fstat(in, &st) != 0)
    usage(strerror(errno));

  if ((st.st_size % IMDB_REC_LEN) != 0)
    usage("database size expected to be multiples to 48");

  if (lseek(in, 0, SEEK_SET) < 0)
    usage(strerror(errno));

  if (read(in, in_buf, IMDB_REC_LEN) != IMDB_REC_LEN)
    usage("can't read header of database");

  if (memcmp(in_buf, "DB of image fingerprints (ver 1)", 32) != 0)
    usage("wrong database header / version mismatch");

  imdb_rec_total = (st.st_size / IMDB_REC_LEN);
  printf("Processing %lu records\n", imdb_rec_total - 1);

  if (lseek(in, IMDB_REC_LEN, SEEK_SET) < 0)
    usage(strerror(errno));

  if (lseek(out, 0, SEEK_SET) < 0)
    usage(strerror(errno));
  if (write(out, header, IMDB_REC_LEN) != IMDB_REC_LEN)
    usage(strerror(errno));

  for (unsigned int block = 0; block <= (imdb_rec_total / BLK_SIZE); block++) {
    rec_first = rec_last = (block * BLK_SIZE) + 1;
    records   = ((rec_first + BLK_SIZE) > imdb_rec_total) ? imdb_rec_total - rec_first : BLK_SIZE;
    rec_last  = rec_first + records;
    printf("* block %u, %4lu records [%lu, %lu]\n",
      block + 1, records, rec_first, rec_last - 1);
    lseek(in,  rec_first * IMDB_REC_LEN, SEEK_SET);
    lseek(out, rec_first * IMDB_REC_LEN, SEEK_SET);
    bytes = read(in, in_buf, IMDB_REC_LEN * records);
    if (bytes != (ssize_t) records * IMDB_REC_LEN) {
      printf("Read size mismatch, expected %lu, got: %i\n",
        IMDB_REC_LEN * records, bytes);
      exit(EXIT_FAILURE);
    }
    memset(out_buf, 0x0, records * IMDB_REC_LEN);
    for (unsigned int i = 0; i < records; i++) {
      src =  &in_buf[i * IMDB_REC_LEN];
      dst = &out_buf[i * IMDB_REC_LEN];
      memcpy(dst +  0, src + 0, sizeof(char) *  1); // usage flag
      memcpy(dst + 16, src + 2, sizeof(char) *  32); // image bitmap
    }
    bytes = write(out, out_buf, IMDB_REC_LEN * records);
    if (bytes != (ssize_t) records * IMDB_REC_LEN) {
      printf("Write size mismatch, expected %lu, got: %i\n",
        IMDB_REC_LEN * records, bytes);
      exit(EXIT_FAILURE); 
    }
  }

  exit(EXIT_SUCCESS);
}
