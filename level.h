#ifndef asteroids_level_H
#define asteroids_level_H

#include "asteroid.h"
#include "asteroids.h"

#define START_ASTEROIDS 3

typedef struct level_t {
  LIST *asteroids;
} LEVEL;

LEVEL *create_level(uint8_t level);
void level_free(LEVEL *level);

#endif /* asteroids_level_H */
