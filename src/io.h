/* Copyright 2014-2017 Alex 'AdUser' Z (ad_user@runbox.com)
 *
 * This file is part of libsimdb
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef HAS_IO_H
#define HAS_IO_H 1

/**
 * @file
 * @brief This file contains definitions of internal i/o functions, used by database
 */

/**
 * @brief Read records from database
 * @param db  Database handle
 * @param start First record number
 * @param records Records count to read
 * @param data Storage for records data (allocated)
 * @retval <0 on error
 * @retval  0 on no records read
 * @retval >0 as records count actually read
 * @note If at least one record read, contents of @a data pointer
 *   must be freed by user
 */
int simdb_read(simdb_t *db, int start, int records, simdb_urec_t **data);

/**
 * @brief Write records to database
 * @param db  Database handle
 * @param start First record number
 * @param records Records count to read
 * @param data Pointer to records data
 * @retval <0 on error
 * @retval  0 on no records written
 * @retval >0 as records count actually written
 */
int simdb_write(simdb_t *db, int start, int records, simdb_urec_t *data);

#endif
