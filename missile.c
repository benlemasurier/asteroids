#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#include "asteroids.h"
#include "missile.h"

void
missile_free(MISSILE *missile)
{
  free(missile->position);
  free(missile->velocity);
  al_destroy_bitmap(missile->sprite);
  free(missile);

  missile = NULL;
}

void
missile_draw(MISSILE *missile)
{
  al_draw_bitmap(
      missile->sprite,
      missile->position->x - (missile->width  / 2),
      missile->position->y - (missile->height / 2),
      DRAWING_FLAGS);
}

void
update_missile(MISSILE *missile, ALLEGRO_TIMER *timer)
{
  if((missile->time + (MISSILE_TTL * FPS)) < al_get_timer_count(timer)) {
    missile->active = false;
    return;
  }

  missile->position->x += missile->velocity->x;
  missile->position->y += missile->velocity->y;
  wrap_position(missile->position);
}

MISSILE *
create_missile(void)
{
  MISSILE *missile  = malloc(sizeof(MISSILE));
  missile->position = malloc(sizeof(VECTOR));
  missile->velocity = malloc(sizeof(VECTOR));

  missile->sprite = al_load_bitmap("data/sprites/missile.png");
  missile->width  = al_get_bitmap_width(missile->sprite);
  missile->height = al_get_bitmap_height(missile->sprite);

  if(!missile->sprite) {
    free(missile->position);
    free(missile->velocity);
    free(missile);
    fprintf(stderr, "failed to create missile sprite.\n");

    return NULL;
  }

  missile->active = false;

  return missile;
}
