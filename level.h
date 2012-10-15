#ifndef asteroids_level_H
#define asteroids_level_H

#include "asteroid.h"
#include "asteroids.h"

typedef struct level_t {
  LIST *asteroids;
} LEVEL;

LEVEL *create_level(int n_asteroids);

#endif /* asteroids_level_H */
