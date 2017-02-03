#ifndef HAS_SIMDB_H
#define HAS_SIMDB_H 1

/**
 * @file
 * @brief Exportable simdb functions, defines & structs
 */

#define SIMDB_VERSION  2  /**< database format version */
#define SIMDB_REC_LEN 48  /**< record length, in bytes */

/**
 * @defgroup SIMDBFlags Database runtime flags
 *  @{ */
#define SIMDB_FLAGS_MASK    0xFF
#define SIMDB_FLAG_WRITE    1 << (0 + 0)  /**< database has write access */
#define SIMDB_FLAG_LOCK     1 << (0 + 1)  /**< use locks for file with write access (only with @ref SIMDB_FLAG_WRITE) */
#define SIMDB_FLAG_LOCKNB   1 << (0 + 2)  /**< same as above, but not wait for lock (only with @ref SIMDB_FLAG_WRITE) */
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
 * @defgroup SIMDBAddModifiers Flags for simdb_record_add() call
 * @{ */
#define SIMDB_ADD_NOREPLACE 1 << (0 + 0)  /**< don't replace existing record */
#define SIMDB_ADD_NOEXTEND  1 << (0 + 1)  /**< don't extend database if @a num greater than existing records count */
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
#define SIMDB_ERR_USAGE       -7 /**< wrong arguments passed */
#define SIMDB_ERR_SAMPLER     -8 /**< given file not an image, damaged or has unsupported format */
#define SIMDB_ERR_LOCK        -9 /**< can't add lock on database file */
/** @} */

/** opaque database handler */
typedef struct _simdb_t simdb_t;

/**
 * search matches
 */
typedef struct {
  int num;         /**< record id */
  float d_ratio;   /**< difference of ratio */
  float d_bitmap;  /**< difference of bitmap */
} simdb_match_t;

/**
 * search parameters
 * d_* fields should have value from 0.0 to 1.0 (0% - 100%)
 */
typedef struct {
  float d_bitmap; /**< max difference of luma bitmaps, default - 7% */
  float d_ratio;  /**< max difference of ratios, default - 7% */
  int limit;      /**< max results */
  int found;      /**< count of found results */
  simdb_match_t *matches;
} simdb_search_t;

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
 * @param mode Database open modes. See @ref SIMDBFlags defines above
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
 * @param code Error code, see @ref SIMDBErrors defines above
 */
const char * simdb_error(int code);

/**
 * @brief Initializes search struct
 * @param search Pointer to search struct
 */
void simdb_search_init(simdb_search_t *search);

/**
 * @brief Free search results
 * @param search Pointer to search struct
 */
void simdb_search_free(simdb_search_t *search);

/**
 * @brief Compare given record in database to other records
 * @param db Database handle
 * @param num Record sample number
 * @param search Search parameters
 * @note If called with non-empty search struct, results will be free()ed automatically
 * @retval >0 if found some matches
 * @retval  0 if nothing found
 * @retval <0 on error
 */
int simdb_search_byid(simdb_t *db, simdb_search_t *search, int num);

/**
 * @brief Compare given file against other records in database
 * @param db Database handle
 * @param file Path to file to compare against database
 * @param search Search parameters
 * @note If called with non-empty search struct, results will be free()ed automatically
 * @retval >0 if found some matches
 * @retval  0 if nothing found
 * @retval <0 on error
 */
int simdb_search_file(simdb_t *db, simdb_search_t *search, const char *file);

/**
 * @brief Checks is record with given number is used
 * @param db  Database handle
 * @param num Record number
 * @returns true if used, false if no record exists or not used
 */
bool simdb_record_used(simdb_t *db, int num);

/**
 * @brief Create an record from image file
 * @param db  Database handle
 * @param num Number of record to add / replace
 * @param path Path to source image
 * @param flags Modifier flags. See a @ref SIMDBAddModifiers group for possible values.
 * @retval <0 on error
 * @retval  0 if @a num > 0, SIMDB_ADD_NOEXPAND flag set, but record not exists
 *         or if @a num > 0, SIMDB_ADD_NOREPLACE flag set, and record already used
 * @retval >0 if record added successfully
 * @note setting @a num to zero means "append to end"
 */
int simdb_record_add(simdb_t *db, int num, const char *path, int flags);

/**
 * @brief Delete a record from database by num
 * @param db  Database handle
 * @param num Number of record to delete
 * @retval <0 on error
 * @retval  0 if record not exists
 * @retval >0 as on success
 */
int simdb_record_del(simdb_t *db, int num);

/**
 * @brief Get record bitmap
 * @param db  Database handle
 * @param num Number of record to get bitmap
 * @param bitmap Pointer to storage for bitmap (allocated)
 * @param side   Pointer to storage for bitmap side length
 * @retval <0 on error
 * @retval  0 if record not exists
 * @retval >0 on success (this is size of allocated bitmap in bytes)
 * @note Don't forget to free() bitmap on success
 */
int simdb_record_bitmap(simdb_t *db, int num, char **map, size_t *side);

/**
 * @brief Get database capacity
 */
int simdb_records_count(simdb_t * const db);

/**
  * @brief Fills buffer 'map' according to records existense in database
  * @param db  Database handle
  * @param map Pointer to storage for generated usage map (allocated)
  * @returns records processed (and also buffer size)
*/
int simdb_usage_map(simdb_t * const db, char ** const map);

/**
 * @brief   Fills buffer 'map' according to records existense in given range
 * @param   db      Database handler
 * @param   map     Pointer to storage for generated usage map (allocated)
 * @param   offset  Start of slice position
 * @param   limit   Slice size
 * @returns Records processed (and also buffer size)
*/
int simdb_usage_slice(simdb_t * const db, char ** const map, int offset, int limit);

#endif /* HAS_SIMDB_H */
