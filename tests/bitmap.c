#include "../src/common.h"
#include "../src/bitmap.h"

int
main() {
  unsigned char a[SIMDB_BITMAP_SIZE];
  unsigned char b[SIMDB_BITMAP_SIZE];
  int ret;

  memset (a, 0x00, sizeof(a));
  memset (b, 0x00, sizeof(b));
  ret = simdb_bitmap_compare(a, b);
  assert(ret == 0);

  memset (a, 0xFE, sizeof(a));
  memset (b, 0xFF, sizeof(b));
  ret = simdb_bitmap_compare(a, b);
  assert(ret == 32);

  memset (a, 0x00, sizeof(a));
  memset (b, 0x55, sizeof(b));
  ret = simdb_bitmap_compare(a, b);
  assert(ret == 128);

  memset (a, 0x00, sizeof(a));
  memset (b, 0xAA, sizeof(b));
  ret = simdb_bitmap_compare(a, b);
  assert(ret == 128);

  memset (a, 0xAA, sizeof(a));
  memset (b, 0x55, sizeof(b));
  ret = simdb_bitmap_compare(a, b);
  assert(ret == 256);

  memset (a, 0x00, sizeof(a));
  memset (b, 0xFF, sizeof(b));
  ret = simdb_bitmap_compare(a, b);
  assert(ret == 256);

  return 0;
}
