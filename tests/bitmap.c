#include "../src/common.h"
#include "../src/bitmap.h"

int
main(int argc, char **argv)
{
  bitmap_t a, b;

  memset (a, 0x00, sizeof(bitmap_t));
  memset (b, 0x00, sizeof(bitmap_t));
  assert(bitmap_compare(a, b) == 0);

  memset (a, 0xFE, sizeof(bitmap_t));
  memset (b, 0xFF, sizeof(bitmap_t));
  assert(bitmap_compare(a, b) == 32);

  memset (a, 0x00, sizeof(bitmap_t));
  memset (b, 0x55, sizeof(bitmap_t));
  assert(bitmap_compare(a, b) == 128);

  memset (a, 0x00, sizeof(bitmap_t));
  memset (b, 0xAA, sizeof(bitmap_t));
  assert(bitmap_compare(a, b) == 128);

  memset (a, 0xAA, sizeof(bitmap_t));
  memset (b, 0x55, sizeof(bitmap_t));
  assert(bitmap_compare(a, b) == 256);

  memset (a, 0x00, sizeof(bitmap_t));
  memset (b, 0xFF, sizeof(bitmap_t));
  assert(bitmap_compare(a, b) == 256);

  return 0;
}
