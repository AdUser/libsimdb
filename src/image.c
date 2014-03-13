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

#include <gd.h>
#include <magic.h>

#include "main.h"
#include "image.h"

gdImagePtr
image_from_file(const char *path, char **errstr)
{
  const char *mimetype;
  magic_t magic;
  char err[256] = "\0";

  FILE *in;
  gdImagePtr (*cb)(FILE *fd);
  gdImagePtr img;

  if ((magic = magic_open(MAGIC_MIME_TYPE)) == NULL) {
    snprintf(err, 256, "%s", magic_error(magic));
    *errstr = err;
    return NULL;
  }

  if (magic_load(magic, NULL) < 0) {
    *errstr = magic_error(magic);
    magic_close(magic);
    return NULL;
  }

  mimetype = magic_file(magic, path);
  if (mimetype == NULL) {
    snprintf(err, 256, "%s", magic_error(magic));
    *errstr = err;
    magic_close(magic);
    return NULL;
  }

  if (strncmp(mimetype, "image/png", 9) == 0) {
    cb = gdImageCreateFromPng;
  } else 
  if (strncmp(mimetype, "image/gif", 9) == 0) {
    cb = gdImageCreateFromGif;
  } else
  if (strncmp(mimetype, "image/jpeg", 10) == 0) {
    cb = gdImageCreateFromJpeg;
  } else {
    snprintf(err, 256, "Can't handle image type '%s'", mimetype);
    *errstr = err;
    return NULL;
  }
  magic_close(magic);

  in = fopen(path, "rb");
  if (in == NULL) {
    snprintf(err, 256, "Can't open file: %s", strerror(errno));
    *errstr = err;
    return NULL;
  }

  img = cb(in);
  fclose(in);

  return img;
}
