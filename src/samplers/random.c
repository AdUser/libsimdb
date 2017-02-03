#include "../common.h"
#include "../record.h"
#include "../simdb.h"

simdb_urec_t *
simdb_record_create(const char * const path) {
  simdb_urec_t *tmp;
  uint8_t pattern;
  uint16_t size;
  float ratio = 0;
  assert(path != NULL);

  (void)(path);

  if ((tmp = calloc(1, sizeof(simdb_urec_t))) == NULL)
    return NULL;

  tmp->used = 0xFF;

  size  = 200 + rand() % 2000; /* [200, 2200] */
  ratio = ((50 + rand() % 100) / (float) 100); /* [0.5, 1.5] */
  tmp->image_w = size;
  tmp->image_h = size * ratio;

  pattern = rand() % 256;
  for (size_t i =  0; i < 16; i += 2)
    tmp->bitmap[i + 0] =  pattern,
    tmp->bitmap[i + 1] = ~pattern;

  pattern = rand() % 256;
  for (size_t i = 16; i < 32; i += 2)
    tmp->bitmap[i + 0] =  pattern,
    tmp->bitmap[i + 1] = ~pattern;

  return tmp;
}
