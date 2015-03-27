#include "common.h"
#include "database.h"
#include "sampler.h"

#include <wand/magick_wand.h>

int
imdb_sample(imdb_rec_t * const rec,
                  char * const source)
{
  MagickWand *wand = NULL;
  MagickPassFail status = MagickPass;
  ExceptionType severity = 0;
  unsigned long w = 0, h = 0;
  char *description = NULL;
  size_t buf_size = 64 * sizeof(char);
  unsigned char *buf = NULL;

  InitializeMagick("/"); /* FIXME */
  wand = NewMagickWand();
  if (status == MagickPass)
    status = MagickReadImage(wand, source);

  if (status == MagickPass)
    w = MagickGetImageHeight(wand);

  if (status == MagickPass)
    h = MagickGetImageHeight(wand);

  /* 2 -> 160 : width, "cols" */
  /* 3 -> 160 : width, "rows" */
  if (status == MagickPass)
    status = MagickSampleImage(wand, 160, 160);

  /* TODO: color maps */

  /* 2 -> 256 : number of colors */
  /* 3 ->   0 : treedepth     -> auto */
  /* 4 ->   0 : dither        -> none */
  /* 5 ->   0 : measure_error -> none */
  if (status == MagickPass)
    status = MagickQuantizeImage(wand, 256, GRAYColorspace, 0, 0, 0);

  /* 2 -> 3 : radius, in pixels */
  /* 3 -> 2 : "for reasonable results, radius should be larger than sigma" */
  if (status == MagickPass)
    status = MagickBlurImage(wand, 3, 2);

  if (status == MagickPass)
    status = MagickNormalizeImage(wand);

  if (status == MagickPass)
    status = MagickEqualizeImage(wand);

  /* 2 -> 16 : width, "cols" */
  /* 3 -> 16 : width, "rows" */
  if (status == MagickPass)
    status = MagickSampleImage(wand, 16, 16);

  /* 2 -> 0.5 : threshold (tune?) */
  if (status == MagickPass)
    status = MagickThresholdImage(wand, 0.5);

  if (status == MagickPass)
    status = MagickSetImageType(wand, BilevelType);

  if (status == MagickPass)
    status = MagickSetImageFormat(wand, "BMP");

  if (status == MagickPass)
    buf = MagickWriteImageBlob(wand, &buf_size);

  if (status != MagickPass) {
    description = MagickGetException(wand, &severity);
    fprintf(stderr, "%03d %.1024s\n", severity, description); /* FIXME */
  }

  DestroyMagickWand(wand);
  DestroyMagick();

  if (status != MagickPass)
    return -1;

  rec->data[REC_OFF_RU] = 1;
  *((uint16_t *) &rec->data[REC_OFF_IW]) = (uint16_t) w;
  *((uint16_t *) &rec->data[REC_OFF_IH]) = (uint16_t) h;
  memcpy(&rec->data[REC_OFF_BM], buf, 32);

  return 0;
}
