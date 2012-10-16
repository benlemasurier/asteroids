#ifndef asteroids_level_H
#define asteroids_level_H

#include "asteroid.h"
#include "asteroids.h"
#include "ship.h"
#include "saucer.h"

#define START_ASTEROIDS 3

typedef struct level_t {
  LIST *asteroids;
  SAUCER *saucer;
} LEVEL;

LEVEL *create_level(uint8_t level);
void level_free(LEVEL *level);
void level_update(LEVEL *level, SHIP *ship, ALLEGRO_TIMER *timer);
#endif /* asteroids_level_H */
