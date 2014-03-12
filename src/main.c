/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#include "main.h"
#include <gdbm.h>

#define DB_BLOCKSIZE 256 * 1024

GDBM_FILE dbf;

int db_open(char *samples, char *files)
{
  dbf = gdbm_open(samples, DB_BLOCKSIZE, GDBM_WRCREAT, 0644, 0);

  if (!dbf) {
    printf("Can't open database: %s\n", gdbm_strerror(gdbm_errno));
    return 0;
  }

  return 1;
}
