#include "common.h"
#include "bitmap.h"
#include "simdb.h"
#include "sample.h"

#include <wand/magick_wand.h>

int
simdb_sample(simdb_rec_t * const rec,
              const char * const source)
{
  MagickWand *wand = NULL;
  MagickPassFail status = MagickPass;
  ExceptionType severity = 0;
  uint16_t w = 0, h = 0;
  char *description = NULL;
  size_t buf_size = 64 * sizeof(char);
  unsigned char *buf = NULL;

  InitializeMagick("/"); /* FIXME */
  wand = NewMagickWand();
  if (status == MagickPass)
    status = MagickReadImage(wand, source);

  if (status == MagickPass)
    w = MagickGetImageWidth(wand);

  if (status == MagickPass)
    h = MagickGetImageHeight(wand);

  /* 2 -> 160 : width, "cols" */
  /* 3 -> 160 : width, "rows" */
  if (status == MagickPass)
    status = MagickSampleImage(wand, 160, 160);

  /* TODO: color maps */

  /* 2 -> 256 : number of colors */
  /* 4 ->   0 : treedepth     -> auto */
  /* 5 ->   0 : dither        -> none */
  /* 6 ->   0 : measure_error -> none */
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

  /* 2 -> 50%, tuned by magick number, see 'magick/image.h' */
  if (status == MagickPass)
    status = MagickThresholdImage(wand, 50.0 * (MaxRGB / 100));

  if (status == MagickPass)
    status = MagickSetImageType(wand, BilevelType);

  if (status == MagickPass)
    status = MagickSetImageFormat(wand, "MONO");

  if (status == MagickPass)
    buf = MagickWriteImageBlob(wand, &buf_size);

#ifdef DEBUG
  if (status == MagickPass) {
    fprintf(stderr, "sample H: %lu\n", MagickGetImageWidth(wand));
    fprintf(stderr, "sample W: %lu\n", MagickGetImageWidth(wand));
    fprintf(stderr, "buf size: %zu\n", buf_size);
    for (unsigned int i = 0; i < buf_size; i++)
      fprintf(stderr, "%02X", buf[i]);
  }
#endif

  if (status == MagickPass) {
    assert(buf_size == BITMAP_SIZE);
    memset(rec->data, 0x0, SIMDB_REC_LEN);
    rec->data[REC_OFF_RU] = 0xFF;
    memcpy(&rec->data[REC_OFF_IW], &w, sizeof(uint16_t));
    memcpy(&rec->data[REC_OFF_IH], &h, sizeof(uint16_t));
    memcpy(&rec->data[REC_OFF_BM], buf, BITMAP_SIZE);
  } else {
    description = MagickGetException(wand, &severity);
    fprintf(stderr, "%03d %.1024s\n", severity, description);
  }

  DestroyMagickWand(wand);
  DestroyMagick();

  return (status == MagickPass) ? 0 : -1;
}
