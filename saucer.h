#ifndef asteroids_saucer_H
#define asteroids_saucer_H

#include "ship.h"
#include "missile.h"

enum {
  SAUCER_SMALL = 0,
  SAUCER_LARGE = 1
};

typedef struct saucer_t {
  uint8_t size;
  unsigned int points;

  uint8_t width;
  uint8_t height;

  float angle;
  VECTOR *position;
  VECTOR *velocity;

  MISSILE *missile;

  ALLEGRO_BITMAP *sprite;
} SAUCER;

void saucer_draw(SAUCER *saucer);
bool saucer_init(void);
SAUCER *saucer_new(uint8_t size);
void saucer_fire(SAUCER *saucer, SHIP *ship, ALLEGRO_TIMER *timer);
void saucer_free(SAUCER *saucer);
void saucer_update(SAUCER *saucer, SHIP *ship, ALLEGRO_TIMER *timer);

#endif /* asteroids_saucer_H */
