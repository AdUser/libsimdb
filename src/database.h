#ifndef HAS_DATABASE_H
#define HAS_DATABASE_H 1

#define IMDB_REC_LEN 48
#define IMDB_VERSION  2

typedef struct {
  int fd;
  int write;
  const char *path;
  const char *errstr;
  unsigned char caps[8];
} imdb_db_t;

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

/**
 * block of sequental records of database
 */
typedef struct {
  uint64_t start;
  size_t records;
  unsigned char *data;
} imdb_block_t;

/**
 * database record
 */
typedef struct {
  uint64_t num;
  unsigned char data[IMDB_REC_LEN];
} imdb_rec_t;

/**
 * search parameters
 * maxdiff_* fields should have value from 0.0 to 1.0 (0% - 100%)
 */
typedef struct {
  uint8_t limit;        /**< max results */
  float maxdiff_bitmap; /**< max difference of luma bitmaps */
  float maxdiff_ratio;  /**< max difference of ratios, default - 7% */
} imdb_search_t;

/**
 * search matches
 */
typedef struct {
  uint64_t num;       /**< record id */
  float diff_ratio;   /**< difference of ratio */
  float diff_bitmap;  /**< difference of bitmap */
} imdb_match_t;

/**
 * @return 1 on success, 0 if record not used
 *          and -1 if record number not exists,
 */
extern int imdb_read_rec (imdb_db_t *db, imdb_rec_t *rec);
extern int imdb_write_rec(imdb_db_t *db, imdb_rec_t *rec);

extern int imdb_read_blk (imdb_db_t *db, imdb_block_t *blk);
extern int imdb_write_blk(imdb_db_t *db, imdb_block_t *blk);

extern int imdb_init (imdb_db_t *db, const char *path);
extern int imdb_open (imdb_db_t *db, const char *path, int write);
extern int imdb_close(imdb_db_t *db);

float
ratio_from_rec_data(unsigned char * const data);

/**
 @returns:
  -1 on error
   0 if nothing found
  >0 if found some matches
 */
extern int
imdb_search(imdb_db_t     * const db,
            imdb_rec_t    * const sample,
            imdb_search_t * const search,
            imdb_match_t  ** matches);

/**
 @returns: number of records in database
*/
uint64_t
imdb_records_count(imdb_db_t * const db);

/**
  @brief   fills buffer 'map' according to records existense in database
  @returns records processed (and also buffer size)
*/
uint64_t
imdb_usage_map(imdb_db_t * const db,
               char     ** const map);

/**
  @brief   fills buffer 'map' according to records existense in given range
  @param   offset  start of slice position
  @param   limit   slice size
  @returns records processed (and also buffer size)
*/
uint16_t
imdb_usage_slice(imdb_db_t * const db,
                 char     ** const map,
                 uint64_t  offset,
                 uint16_t  limit);

#endif
