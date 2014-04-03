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

int
image_bitmap(bitmap_t *bitmap, image_t *image, char **errstr)
{
  gdImagePtr src, dst = (NULL, NULL);
  int16_t res_x, res_y = (160, 160);
  int16_t i, j;
  int oldcolor, newcolor, index;

  assert((dst = gdImageCreateTrueColor(res_x, res_y)) != NULL);

  /* resample to 160x160 */
  gdImageCopyResampled(dst, image->data,
    0,            0,             /* dstX, dstY */
    0,            0,             /* srcX, srcY */
    res_x,        res_y,         /* dstW, dstH */
    image->res_x, image->res_y); /* srcW, srcH */

  /* grayscale */
  gdFree(src);
  src = dst;
  assert((dst = gdImageCreateTrueColor(res_x, res_y)) != NULL);
  for (i = 0; i < res_x; i++) {
    for (j = 0; j < res_x; j++) {
      /* WTF: begin */
      oldcolor = gdImageGetPixel(src, i, j);
      newcolor = 0;
      newcolor += 2 * gdImageGreen (src, oldcolor);
      newcolor += 3 * gdImageRed   (src, oldcolor);
      newcolor += 4 * gdImageBlue  (src, oldcolor);
      newcolor /= 9;
      /* WTF: end */
      if ((index = gdImageColorExact(dst,newcolor,newcolor,newcolor)) == -1) {
        index = gdImageColorAllocate(dst,newcolor,newcolor,newcolor);
      }
      gdImageSetPixel(dst, i, j, index);
    }
  }

  /* blur */
  gdFree(src);
  src = dst;
  assert((dst = gdImageCreateTrueColor(res_x, res_y)) != NULL);

  /* normalize (intensity) */
  /* equalize (contrast) (sharpen?) */
  /* resample to 16x16 */
  /* convert to b&w (treshold?) */
  /* make bitmap */

  return 0;
}
