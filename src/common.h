/* Copyright 2014-2017 Alex 'AdUser' Z (ad_user@runbox.com)
 *
 * This file is part of libsimdb
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/**
 * @file
 * Common include headers
 */

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>

/** frees memory and sets pointer to zero */
#define FREE(ptr) \
  free((ptr)); (ptr) = NULL;
