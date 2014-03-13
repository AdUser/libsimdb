#include <gd.h>

#include "main.h"
#include "image.h"

int main(int argc, char **argv) {
  image_t *img = NULL;
  char *err;

  img = image_from_file("test.png", &err);
  if (img == NULL) {
    printf("%s\n", err);
    exit(EXIT_FAILURE);
  }
  exit(EXIT_SUCCESS);
}
