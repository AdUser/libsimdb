#define SAMPLE_SIZE sizeof(uint16_t)

typedef uint16_t image_bitmap[16];

int bitmap_compare(image_bitmap * const a, image_bitmap * const b);
int image_load(void);
