#ifndef HAS_IMAGE_H
typedef struct
{
  gdImagePtr data;
  char *mime;
} image_t;

image_t *
image_from_file(const char *path, char **errstr);
#endif
#define HAS_IMAGE_H
