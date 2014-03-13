typedef struct
{
  gdImagePtr data;
  char *mime;
} image_t;

image_t *
image_from_file(const char *path, char **errstr);
