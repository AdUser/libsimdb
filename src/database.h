#ifndef HAS_DATABASE_H
#define HAS_DATABASE_H 1

#define SIMDB_REC_LEN 48
#define SIMDB_VERSION  2

/* runtime flags */
#define SIMDB_FLAG_WRITE    1 << (0 + 0)

/* database capabilities */
#define SIMDB_CAP_BITMAP    1 << (8 + 0)
#define SIMDB_CAP_COLORS    1 << (8 + 1)
#define SIMDB_CAP_RATIO     1 << (8 + 2)
/* 3 used, 5 reserved */

#define SIMDB_SUCCESS          0
/* database errors */
#define SIMDB_ERR_SYSTEM      -1 /* see errno for details */
#define SIMDB_ERR_OOM         -2 /* can't allocate memory */
#define SIMDB_ERR_CORRUPTDB   -3 /* empty or currupted database */
#define SIMDB_ERR_WRONGVERS   -4 /* database version mismatch */
#define SIMDB_ERR_NXRECORD    -5 /* no such record in database */
#define SIMDB_ERR_READONLY    -6 /* database opened in read-only mode */

typedef struct {
  int fd;
  int flags;
  char path[PATH_MAX];
} simdb_t;

/**
  Database header format - fixed length, 48 bytes

 0-15 : "IMDB vXX, CAPS: "
16-23 : capabilities, terminated with ';'
24-48 : padding with null's
*/

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
} simdb_block_t;

/**
 * database record
 */
typedef struct {
  uint64_t num;
  unsigned char data[SIMDB_REC_LEN];
} simdb_rec_t;

/**
 * search parameters
 * maxdiff_* fields should have value from 0.0 to 1.0 (0% - 100%)
 */
typedef struct {
  uint8_t limit;        /**< max results */
  float maxdiff_bitmap; /**< max difference of luma bitmaps */
  float maxdiff_ratio;  /**< max difference of ratios, default - 7% */
} simdb_search_t;

/**
 * search matches
 */
typedef struct {
  uint64_t num;       /**< record id */
  float diff_ratio;   /**< difference of ratio */
  float diff_bitmap;  /**< difference of bitmap */
} simdb_match_t;

/**
 * @returns 1 on success, 0 if record missing and <0 on error
 */
int simdb_read_rec (simdb_t *db, simdb_rec_t *rec);
int simdb_write_rec(simdb_t *db, simdb_rec_t *rec);

int simdb_read_blk (simdb_t *db, simdb_block_t *blk);
int simdb_write_blk(simdb_t *db, simdb_block_t *blk);

/**
 * @brief Creates empty database at given path
 * @param path Path to database
 * @returns true on success, and false on error
 * @note See errno value for details
 * @todo 2nd arg: caps
 */
bool simdb_create(const char *path);

/**
 * @brief Open database at given path
 * @param path Path to database
 * @param mode Database open modes. See SIMDB_FLAG_* defines above
 * @param error Pointer to error code storage
 * @returns Pointer to database handle on success, NULL on error
 * @note use @a simdb_error() to get error description
 */
simdb_t * simdb_open(const char *path, int mode, int *error);

/**
 * @brief Close database and free associated resources
 * @param db Database handle
 */
void simdb_close(simdb_t *db);

/**
 * @brief Get error desctiption by error code
 * @param error Error code, see SIMDB_ERR_* defines above
 */
const char * simdb_error(int code);

float
ratio_from_rec_data(unsigned char * const data);

/**
 @returns: >0 if found some matches, 0 if nothing found, <0 on error
 */
int
simdb_search(simdb_t         * const db,
             simdb_rec_t     * const sample,
             simdb_search_t  * const search,
             simdb_match_t  ** matches);

/**
 @returns: number of records in database
*/
uint64_t
simdb_records_count(simdb_t * const db);

/**
  @brief   fills buffer 'map' according to records existense in database
  @returns records processed (and also buffer size)
*/
uint64_t
simdb_usage_map(simdb_t * const db,
               char     ** const map);

/**
  @brief   fills buffer 'map' according to records existense in given range
  @param   offset  start of slice position
  @param   limit   slice size
  @returns records processed (and also buffer size)
*/
uint16_t
simdb_usage_slice(simdb_t * const db,
                 char     ** const map,
                 uint64_t  offset,
                 uint16_t  limit);

#endif
