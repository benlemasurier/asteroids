#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#include "asteroids.h"
#include "util.h"
#include "asteroid.h"

ASTEROID *
create_asteroid(uint8_t size)
{
  ASTEROID *asteroid = malloc(sizeof(ASTEROID));

  asteroid->position = malloc(sizeof(VECTOR));
  asteroid->velocity = malloc(sizeof(VECTOR));

  asteroid->size  = size;
  asteroid->angle = rand_f(0.0, 360.0);
  asteroid->position->x = rand_f(1.0, SCREEN_W);
  asteroid->position->y = rand_f(1.0, SCREEN_H);
  asteroid->velocity->x = (float)   sin(deg2rad(asteroid->angle))  * rand_f(0.1, 1.2);
  asteroid->velocity->y = (float) -(cos(deg2rad(asteroid->angle))) * rand_f(0.1, 1.2);

  switch(size) {
    case ASTEROID_LARGE:
      asteroid->points = ASTEROID_LARGE_POINTS;
      break;
    case ASTEROID_MEDIUM:
      asteroid->points = ASTEROID_MEDIUM_POINTS;
      break;
    case ASTEROID_SMALL:
      asteroid->points = ASTEROID_SMALL_POINTS;
      break;
  }

  if((asteroid->sprite = load_asteroid_sprite(size, asteroid->angle)) == NULL) {
    free(asteroid->position);
    free(asteroid->velocity);
    free(asteroid);

    return NULL;
  }

  asteroid->width  = al_get_bitmap_width(asteroid->sprite);
  asteroid->height = al_get_bitmap_height(asteroid->sprite);

  return asteroid;
}

void
asteroid_draw(ASTEROID *asteroid)
{
  al_draw_bitmap(
      asteroid->sprite,
      asteroid->position->x - (asteroid->width  / 2),
      asteroid->position->y - (asteroid->height / 2),
      DRAWING_FLAGS);
}

void
asteroid_update(ASTEROID *asteroid)
{
  asteroid->position->x += asteroid->velocity->x;
  asteroid->position->y += asteroid->velocity->y;
  wrap_position(asteroid->position);
}

void
asteroid_free(ASTEROID *asteroid)
{
  free(asteroid->position);
  free(asteroid->velocity);
  free(asteroid);

  asteroid = NULL;
}
