#include "../common.h"
#include "../bitmap.h"
#include "../record.h"
#include "../simdb.h"

#include <wand/magick_wand.h>

simdb_urec_t *
simdb_record_create(const char * const path) {
  MagickWand *wand = NULL;
  MagickPassFail status = MagickPass;
  uint16_t w = 0, h = 0;
  size_t buf_size = 64 * sizeof(char);
  unsigned char *buf = NULL;
  simdb_urec_t *rec = NULL;

  assert(path != NULL);

  InitializeMagick("/");
  wand = NewMagickWand();
  if (status == MagickPass)
    status = MagickReadImage(wand, path);

  if (status == MagickPass)
    w = MagickGetImageWidth(wand);

  if (status == MagickPass)
    h = MagickGetImageHeight(wand);

  /* 2 -> 160 : width, "cols" */
  /* 3 -> 160 : width, "rows" */
  if (status == MagickPass)
    status = MagickSampleImage(wand, 160, 160);

  /** TODO: color maps */

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
    assert(buf_size == SIMDB_BITMAP_SIZE);
    if ((rec = calloc(1, sizeof(simdb_urec_t))) != NULL) {
      rec->used = 0xFF;
      rec->image_w = w;
      rec->image_h = h;
      memcpy(rec->bitmap, buf, SIMDB_BITMAP_SIZE);
    }
#ifdef DEBUG
  } else {
    ExceptionType severity = 0;
    char *description = MagickGetException(wand, &severity);
    fprintf(stderr, "%03d %.1024s\n", severity, description);
#endif
  }

  DestroyMagickWand(wand);
  DestroyMagick();

  return rec;
}
