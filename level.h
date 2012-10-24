#ifndef asteroids_level_H
#define asteroids_level_H

#include "asteroid.h"
#include "asteroids.h"
#include "ship.h"
#include "saucer.h"

#define LEVEL_MAX 10
#define MAX_START_ASTEROIDS 14

typedef struct level_t {
  bool playable;
  SAUCER *saucer;
  LIST *asteroids;
  uint8_t number;
  int64_t saucer_seen; /* time since saucer was last seen */
  unsigned long int score;
} LEVEL;

LEVEL *level_create(uint8_t level_number, unsigned long int score);
void level_draw(LEVEL *level);
void level_free(LEVEL *level);
bool level_init(void);
void level_update(LEVEL *level, SHIP *ship, ALLEGRO_TIMER *timer);
#endif /* asteroids_level_H */
