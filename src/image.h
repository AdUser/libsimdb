#ifndef HAS_IMAGE_H
typedef struct
{
  gdImagePtr data;
  uint16_t res_x;
  uint16_t res_y;
  char *mime;
} image_t;
/**
 * @returns: image_t * on success, NULL otherwise
 */
image_t *
image_from_file(const char *path, const char **errstr);

/**
 * @returns: 0 on success, -1 on error
 */
int
image_bitmap(bitmap_t *bitmap, image_t *image, char **errstr);
#endif
#define HAS_IMAGE_H 1
