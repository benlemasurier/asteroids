#ifndef asteroids_missile_H
#define asteroids_missile_H

#include "asteroids.h"

#define MISSILE_TTL   1

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

void missile_free(MISSILE *missile);
void missile_draw(MISSILE *missile);
void update_missile(MISSILE *missile, ALLEGRO_TIMER *timer);
MISSILE *missile_create(void);

#endif /* asteroids_missile_H */
