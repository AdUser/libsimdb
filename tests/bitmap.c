#include "main.h"
#include "bitmap.h"

int
main(int argc, char **argv)
{
  bitmap a, b;

  memset (&a, 0x00, 16 * sizeof(uint16_t));
  memset (&b, 0x00, 16 * sizeof(uint16_t));
  assert(bitmap_compare(&a, &b) == 0);

  memset (&a, 0xFE, 16 * sizeof(uint16_t));
  memset (&b, 0xFF, 16 * sizeof(uint16_t));
  assert(bitmap_compare(&a, &b) == 32);

  memset (&a, 0x00, 16 * sizeof(uint16_t));
  memset (&b, 0x55, 16 * sizeof(uint16_t));
  assert(bitmap_compare(&a, &b) == 128);

  memset (&a, 0x00, 16 * sizeof(uint16_t));
  memset (&b, 0xAA, 16 * sizeof(uint16_t));
  assert(bitmap_compare(&a, &b) == 128);

  memset (&a, 0xAA, 16 * sizeof(uint16_t));
  memset (&b, 0x55, 16 * sizeof(uint16_t));
  assert(bitmap_compare(&a, &b) == 256);

  memset (&a, 0x00, 16 * sizeof(uint16_t));
  memset (&b, 0xFF, 16 * sizeof(uint16_t));
  assert(bitmap_compare(&a, &b) == 256);

  return 0;
}
