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

/** @defgroup MemoryMgmt Memory management macro
 * @{*/
/** frees memory and sets pointer to zero */
#define FREE(ptr) \
  free((ptr)); (ptr) = NULL;

/** allocates memory, and checks is it really allocated */
#define CALLOC(ptr, nmemb, size) \
  assert(((ptr) = calloc((nmemb), (size))) != NULL);
/** @} */
