#ifndef asteroids_missile_H
#define asteroids_missile_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

#include "list.h"
#include "asteroid.h"
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
  ALLEGRO_SAMPLE *sample;
} MISSILE;

bool missile_asteroid_collision(MISSILE *m, ASTEROID *a);
MISSILE *missile_create(void);
void missile_free(MISSILE *missile);
void missile_fire(MISSILE *missile, VECTOR *position, float angle, ALLEGRO_TIMER *timer);
void missile_draw(MISSILE *missile);
void missile_draw_list(LIST *missiles);
bool missile_init(void);
void missile_update(MISSILE *missile, ALLEGRO_TIMER *timer);
void missile_shutdown(void);

#endif /* asteroids_missile_H */
