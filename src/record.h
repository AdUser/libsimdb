#ifndef HAS_RECORD_H
#define HAS_RECORD_H 1

#include "bitmap.h"
#include "simdb.h"

/**
 * @file
 * @brief Routines for work with record
 */

/** struct for packed record usage */
typedef struct __attribute__((__packed__)) {
  uint8_t used;      /**< record is used */
  uint8_t clevel_r;  /**< color level: red   */
  uint8_t clevel_g;  /**< color level: green */
  uint8_t clevel_b;  /**< color level: blue  */
  uint16_t image_w;  /**< image width  */
  uint16_t image_h;  /**< image height */
  unsigned char __unused[8]; /**< padding */
  unsigned char bitmap[SIMDB_BITMAP_SIZE]; /**< image luma bitmap */
} simdb_urec_t;

/** compile-time check for packed struct length */
typedef char size_mismatch_for__simdb_urec_t[(sizeof(simdb_urec_t) == SIMDB_REC_LEN) * 2 - 1];

/**
 * @brief Creates metadata record from given image
 * @param path Path to source image
 * @returns Pointer to allocated record or NULL on error
 */
simdb_urec_t * simdb_record_create(const char * const path);

#endif /* RECORD_H */
