#include "../src/common.h"
#include "../src/bitmap.h"

int
main() {
  unsigned char a[BITMAP_SIZE], b[BITMAP_SIZE];

  memset (a, 0x00, sizeof(a));
  memset (b, 0x00, sizeof(b));
  assert(bitmap_compare(a, b) == 0);

  memset (a, 0xFE, sizeof(a));
  memset (b, 0xFF, sizeof(b));
  assert(bitmap_compare(a, b) == 32);

  memset (a, 0x00, sizeof(a));
  memset (b, 0x55, sizeof(b));
  assert(bitmap_compare(a, b) == 128);

  memset (a, 0x00, sizeof(a));
  memset (b, 0xAA, sizeof(b));
  assert(bitmap_compare(a, b) == 128);

  memset (a, 0xAA, sizeof(a));
  memset (b, 0x55, sizeof(b));
  assert(bitmap_compare(a, b) == 256);

  memset (a, 0x00, sizeof(a));
  memset (b, 0xFF, sizeof(b));
  assert(bitmap_compare(a, b) == 256);

  return 0;
}
