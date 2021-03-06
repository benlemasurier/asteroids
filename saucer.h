#ifndef asteroids_saucer_H
#define asteroids_saucer_H

#include "ship.h"
#include "missile.h"
#include "asteroid.h"

#define SAUCER_TTL 10

enum {
  SAUCER_SMALL = 0,
  SAUCER_LARGE = 1
};

typedef struct saucer_t {
  uint8_t size;
  unsigned int points;
  uint8_t level;
  int64_t entry_time;

  uint8_t width;
  uint8_t height;

  float angle;
  VECTOR *position;
  VECTOR *velocity;
  VECTOR *exit;

  MISSILE *missile;

  ALLEGRO_BITMAP *sprite;
} SAUCER;

bool saucer_asteroid_collision(SAUCER *s, ASTEROID *a);
void saucer_draw(SAUCER *saucer);
bool saucer_init(void);
void saucer_exit(SAUCER *saucer);
SAUCER *saucer_new(uint8_t size, uint8_t level, ALLEGRO_TIMER *timer);
void saucer_fire(SAUCER *saucer, SHIP *ship, ALLEGRO_TIMER *timer);
void saucer_free(SAUCER *saucer);
SAUCER *saucer_update(SAUCER *saucer, SHIP *ship, ALLEGRO_TIMER *timer);

#endif /* asteroids_saucer_H */
