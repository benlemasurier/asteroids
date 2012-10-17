#ifndef asteroids_level_H
#define asteroids_level_H

#include "asteroid.h"
#include "asteroids.h"
#include "ship.h"
#include "saucer.h"

#define LEVEL_MAX 10
#define MAX_START_ASTEROIDS 14

typedef struct level_t {
  LIST *asteroids;
  SAUCER *saucer;
  uint8_t number;
  int64_t saucer_seen; /* time since saucer was last seen */
} LEVEL;

LEVEL *level_create(uint8_t level_number);
void level_free(LEVEL *level);
void level_update(LEVEL *level, SHIP *ship, ALLEGRO_TIMER *timer);
#endif /* asteroids_level_H */
