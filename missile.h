#ifndef asteroids_missile_H
#define asteroids_missile_H

#include "asteroids.h"

#define MISSILE_TTL   1.5

typedef struct missile_t {
  int width;
  int height;
  bool active;
  int64_t time;

  float angle;
  VECTOR *position;
  VECTOR *velocity;

  ALLEGRO_BITMAP *sprite;
} MISSILE;

MISSILE *missile_create(void);
void missile_free(MISSILE *missile);
void missile_draw(MISSILE *missile);
void missile_update(MISSILE *missile, ALLEGRO_TIMER *timer);

#endif /* asteroids_missile_H */
