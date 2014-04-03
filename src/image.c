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

#include "common.h"
#include <magick/api.h>
#include "database.h"
#include "image.h"

int image_load(image_t *img, const char *path)
{
  ExceptionInfo ex;

  GetExceptionInfo(&ex);

  img.info = CloneImageInfo((ImageInfo *) NULL);
  strcpy(img.info->filename, argv[1]);

  img.data = ReadImage(img.info, &ex);

  if (ex.severity != UndefinedException)
    CatchException(&ex);

  return 0;
}

int image_sample(rec_t *sample, image_t *img)
{
  ExceptionInfo ex;
  QuantizeInfo qi;

  ImageInfo info;
  Image *sampled;
  Image *blurred;

  GetExceptionInfo(&ex);

  memset(img, 0x0, sizeof(image_t));

  sampled = SampleImage(img.data, 160, 160, &ex);

  GetQuantizeInfo(&qi);
  QuantizeImage(&qi, sampled)

  blurred = BlurImage(sampled, 3.0, 99, &ex); /* reduce sigma? */
  DestroyImage(sampled);

  ret = GetImageStatistics(blurred, &stat, &ex);

  NormalizeImage(blurred);
  EqualizeImage(blurred);

  sampled = SampleImage(img.data, 16, 16, &ex);
  DestroyImage(blurred);

  ret = ThresholdImage(sampled, /* const double threshold */ 0);

  GetImageInfo(&info);

  SetImageAttribute(sampled, "magick", "mono");
  SetImageType(sampled, BilevelType);
  (sample.data + OFF_BITMAP) = ImageToBlob(&info, sampled, BITMAP_SIZE, &ex);

  return 0;
}
