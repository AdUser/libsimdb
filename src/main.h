#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>

extern int db_open(char *samples, char *files);
extern int db_close(void);

extern int image_add(uint64_t, char *path);
extern int image_del(uint64_t);
extern int image_exists(uint64_t);
/*extern int image_find(uint64_t, rec_t **data, size_t limit);*/
