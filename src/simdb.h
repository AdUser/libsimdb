#ifndef HAS_DATABASE_H
#define HAS_DATABASE_H 1

/**
 * @file
 * @brief Exportable simdb functions, defines & structs
 */

#define SIMDB_VERSION  2  /**< database format version */
#define SIMDB_REC_LEN 48  /**< record length, in bytes */

/**
 * @defgroup SIMDBFlags Database runtime flags
 *  @{ */
#define SIMDB_FLAG_WRITE    1 << (0 + 0)  /**< database has write access */
/** @} */

/**
 * @defgroup SIMDBCaps Database capabilities
 * @{ */
#define SIMDB_CAP_BITMAP    1 << (8 + 0)  /**< database can compare images by luma bitmaps */
#define SIMDB_CAP_COLORS    1 << (8 + 1)  /**< database can compare images by color levels */
#define SIMDB_CAP_RATIO     1 << (8 + 2)  /**< database can compare images by ratio */
/* 3 used, 5 reserved */
/** @} */

/**
 * @defgroup SIMDBErrors Database error codes
 * @{ */
#define SIMDB_SUCCESS          0 /**< success */
#define SIMDB_ERR_SYSTEM      -1 /**< see errno for details */
#define SIMDB_ERR_OOM         -2 /**< can't allocate memory */
#define SIMDB_ERR_CORRUPTDB   -3 /**< empty or corrupted database */
#define SIMDB_ERR_WRONGVERS   -4 /**< database version mismatch */
#define SIMDB_ERR_NXRECORD    -5 /**< no such record in database */
#define SIMDB_ERR_READONLY    -6 /**< database opened in read-only mode */
/** @} */

/** opaque database handler */
typedef struct _simdb_t simdb_t;

/**
 * @defgroup SIMDBRecOffsets Sample data parts offset in database record
 * @{ */
#define REC_OFF_RU  0 /**< record is used */
#define REC_OFF_CR  1 /**< color level: red */
#define REC_OFF_CG  2 /**< color level: green */
#define REC_OFF_CB  3 /**< color level: blue */
#define REC_OFF_IW  4 /**< image width  */
#define REC_OFF_IH  6 /**< image height */
#define REC_OFF_BM 16 /**< image bitmap */
/** @} */

/**
 * block of sequental records of database
 */
typedef struct {
  uint64_t start;      /**< first record number of block */
  size_t records;      /**< records count */
  unsigned char *data; /**< raw records data */
} simdb_block_t;

/**
 * database record
 */
typedef struct {
  uint64_t num;                      /**< record number in database */
  unsigned char data[SIMDB_REC_LEN]; /**< record raw data */
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
 * @param code Error code, see SIMDB_ERR_* defines above
 */
const char * simdb_error(int code);

/**
 * @brief Search compare given record in database to other images
 * @param db Database handle
 * @param sample Record sample
 * @param search Search parameters
 * @param matches Pointer to storage for found matches
 * @retval >0 if found some matches
 * @retval  0 if nothing found
 * @retval <0 on error
 */
int
simdb_search(simdb_t         * const db,
             simdb_rec_t     * const sample,
             simdb_search_t  * const search,
             simdb_match_t  ** matches);

/**
 * @brief Read single record from database
 * @param db  Database handle
 * @param rec Record struct
 * @note @a num member of @a rec struct should be set
 * @retval  1 on success
 * @retval  0 if record exists but not used,
 * @retval <0 on error (system error or record missing)
 */
int simdb_record_read(simdb_t *db, simdb_rec_t *rec);
/**
 * @brief Write single record to database
 * @param db  Database handle
 * @param rec Record struct to write
 * @note @a num member of @a rec struct should be set
 * @retval  1 on success
 * @retval <0 on error (system error or record missing)
 */
int simdb_record_write(simdb_t *db, simdb_rec_t *rec);

/**
 * @brief Read continious records block from database
 * @param db  Database handle
 * @param blk Block struct
 * @note @a start and @a records members of @a blk struct should be set
 * @retval >=0 As number of records actually read
 * @retval  <0 on error
 */
int simdb_block_read(simdb_t *db, simdb_block_t *blk);

/**
 * @brief Get database capacity
 */
uint64_t
simdb_records_count(simdb_t * const db);

/**
  @brief   Fills buffer 'map' according to records existense in database
  @returns records processed (and also buffer size)
*/
uint64_t
simdb_usage_map(simdb_t * const db,
               char     ** const map);

/**
 * @brief   Fills buffer 'map' according to records existense in given range
 * @param   db      Database handler
 * @param   map     Pointer for storing allocated usage map
 * @param   offset  Start of slice position
 * @param   limit   Slice size
 * @returns Records processed (and also buffer size)
*/
uint16_t
simdb_usage_slice(simdb_t  * const db,
                 char     ** const map,
                 uint64_t  offset,
                 uint16_t  limit);

#endif