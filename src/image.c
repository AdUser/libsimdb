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

#include "common.h"
#include "bitmap.h"
#include "image.h"

image_t *
image_from_file(const char *path, const char **errstr)
{
  const char *mimetype;
  magic_t magic;
  char err[256] = "\0";

  FILE *in;
  gdImagePtr (*cb)(FILE *fd);
  image_t *img = NULL;

  CALLOC(img, 1, sizeof(image_t));

  if ((magic = magic_open(MAGIC_MIME_TYPE)) == NULL) {
    snprintf(err, 256, "%s", magic_error(magic));
    *errstr = err;
    FREE(img);
    return NULL;
  }

  if (magic_load(magic, NULL) < 0) {
    *errstr = magic_error(magic);
    magic_close(magic);
    FREE(img);
    return NULL;
  }

  mimetype = magic_file(magic, path);
  if (mimetype == NULL) {
    snprintf(err, 256, "%s", magic_error(magic));
    *errstr = err;
    magic_close(magic);
    FREE(img);
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
    FREE(img);
    return NULL;
  }
  STRNDUP(img->mime, mimetype, 16);
  magic_close(magic);

  in = fopen(path, "rb");
  if (in == NULL) {
    snprintf(err, 256, "Can't open file: %s", strerror(errno));
    *errstr = err;
    FREE(img);
    return NULL;
  }

  img->data = cb(in);
  fclose(in);

  img->res_x = gdImageSX(img->data);
  img->res_y = gdImageSY(img->data);

  return img;
}
