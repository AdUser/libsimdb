#ifndef HAS_IMAGE_H
typedef struct
{
  ImageInfo *info
  Image *data;
  const char *errstr;
} image_t;

int image_load(image_t *img, const char *path);

#endif
#define HAS_IMAGE_H 1
