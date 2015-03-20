#ifndef HAS_DATABASE_H
#define HAS_DATABASE_H 1

#define IMDB_REC_LEN 48
#define IMDB_VERSION  2
#define OPEN_FLAGS O_CREAT | O_RDWR

typedef struct {
  int fd;
  const char *path;
  const char *errstr;
  unsigned char caps[8];
} imdb_t;

/**
  Database header format - fixed length, 48 bytes

 0-15 : "IMDB vXX, CAPS: "
16-23 : capabilities, terminated with ';'
24-48 : padding with null's
*/

#define CAP_OFF_BITMAP  0
#define CAP_OFF_COLORS  1
#define CAP_OFF_RATIO   2
/* 3 used, 5 reserved */

/**
  Database record format - fixed length, 48 bytes

 # | off | len | description
---+-----+-----+-------------------------------------------------------
 1 |   0 |   1 | record is used
 2 |   1 |   1 | overall level of color: R--
 3 |   2 |   1 | overall level of color: -G-
 4 |   3 |   1 | overall level of color: --B
 5 |   4 |   2 | image width
 6 |   6 |   2 | image height
 - |   8 |   8 | reserved for future use
 7 |  16 |  32 | bitmap, each 2 bytes is row of monochrome image 16x16

field |  12345 6 +       7
map   |  XRGBWWHH________MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
sect  |  [     0-15     ][    16-31     ][    32-48     ]
*/

#define REC_OFF_RU  0 /* record is used */
#define REC_OFF_CR  1 /* color level: red */
#define REC_OFF_CG  2 /* color level: green */
#define REC_OFF_CB  3 /* color level: blue */
#define REC_OFF_IW  4 /* image width  */
#define REC_OFF_IH  6 /* image height */
#define REC_OFF_BM 16 /* image bitmap */

typedef struct {
  uint64_t start;
  size_t records;
  unsigned char *data;
} imdb_block_t;

typedef struct {
  uint64_t num;
  unsigned char data[IMDB_REC_LEN];
} imdb_rec_t;

typedef struct {
  uint64_t limit;
  float tresh_bitmap;
  float tresh_ratio;
} imdb_search_t;

typedef struct {
  uint64_t num;
  float diff;
} imdb_match_t;

extern int imdb_read_rec (imdb_t *db, imdb_rec_t *rec);
extern int imdb_write_rec(imdb_t *db, imdb_rec_t *rec);

extern int imdb_read_blk (imdb_t *db, imdb_block_t *blk);
extern int imdb_write_blk(imdb_t *db, imdb_block_t *blk);

extern int imdb_read_list (imdb_t *db, imdb_rec_t *list, size_t list_len);
extern int imdb_write_list(imdb_t *db, imdb_rec_t *list, size_t list_len);

extern int imdb_open (imdb_t *db, const char *path);
extern int imdb_close(imdb_t *db);

/**
 @returns:
  -1 on error
   0 if nothing found
  >0 if found some matches
 */
extern int
imdb_search(imdb_t        * const db,
            imdb_rec_t    * const sample,
            imdb_search_t * const search,
            imdb_match_t  **matches);

#endif
