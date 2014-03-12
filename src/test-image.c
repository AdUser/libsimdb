#include "main.h"
#include "image.h"

int main(int argc, char **argv)
{
  int i = 0;
  image_bitmap a, b;

  for (i = 0; i < 16; i++) {
    a[i] = 0x0000;
    b[i] = 0xFFFF;
  }

  printf("%u\n", bitmap_compare(&a, &b));

  return 1;
}
