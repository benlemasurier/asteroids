#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#include "util.h"
#include "asteroids.h"
#include "missile.h"
#include "ship.h"
#include "saucer.h"

#define SMALL_POINTS 1000
#define LARGE_POINTS 250 // FIXME: is this correct?

static ALLEGRO_BITMAP *small_sprite = NULL;
static ALLEGRO_BITMAP *large_sprite = NULL;

void
saucer_draw(SAUCER *saucer)
{
  assert(saucer);
  al_draw_bitmap(
      saucer->sprite,
      saucer->position->x,
      saucer->position->y,
      DRAWING_FLAGS);

  if(saucer->missile->active)
    missile_draw(saucer->missile);
}

bool
saucer_init(void)
{
  if((small_sprite = al_load_bitmap("data/sprites/saucer/saucer-small.png")) == NULL) {
    fprintf(stderr, "failed to load small saucer sprite\n");
    return false;
  }

  if((large_sprite = al_load_bitmap("data/sprites/saucer/saucer-large.png")) == NULL) {
    fprintf(stderr, "failed to load large saucer sprite\n");
    return false;
  }

  return true;
}

SAUCER *
saucer_new(uint8_t size)
{
  assert(small_sprite);
  assert(large_sprite);

  SAUCER *saucer   = malloc(sizeof(SAUCER));
  saucer->position = malloc(sizeof(VECTOR));
  saucer->velocity = malloc(sizeof(VECTOR));
  saucer->position->x = rand_f(1.0, SCREEN_W);
  saucer->position->y = rand_f(1.0, SCREEN_H);
  saucer->angle       = rand_f(0.0, 360.0);
  saucer->velocity->x = (float)   sin(deg2rad(saucer->angle));
  saucer->velocity->y = (float) -(cos(deg2rad(saucer->angle)));
  saucer->missile     = create_missile();

  saucer->size = size;
  switch(saucer->size) {
    case SAUCER_SMALL:
      saucer->sprite = small_sprite;
      saucer->points = SMALL_POINTS;
      break;
    case SAUCER_LARGE:
      saucer->sprite = large_sprite;
      saucer->points = LARGE_POINTS;
      break;
  }

  saucer->width  = al_get_bitmap_width(saucer->sprite);
  saucer->height = al_get_bitmap_height(saucer->sprite);

  return saucer;
}

void
saucer_fire(SAUCER *saucer, SHIP *ship, ALLEGRO_TIMER *timer)
{
  MISSILE *missile = saucer->missile;

  /* missiles in use? */
  if(missile->active)
    return;

  /* FIXME: testing */
  missile->active = true;
  missile->position->x = saucer->position->x + (saucer->width  / 2);
  missile->position->y = saucer->position->y + (saucer->height / 2);
  missile->time = al_get_timer_count(timer);

  /* calculate the (FIXME: exact) angle to hit the ship */
  missile->angle = get_angle(ship->position->x, ship->position->y, missile->position->x, missile->position->y);
  missile->angle = rad2deg(missile->angle);
  missile->angle = missile->angle - 90.0;

  missile->velocity->x = (float)   sin(deg2rad(missile->angle))  * MISSILE_SPEED;
  missile->velocity->y = (float) -(cos(deg2rad(missile->angle))) * MISSILE_SPEED;
}

void
saucer_free(SAUCER *saucer)
{
  free(saucer->position);
  free(saucer->velocity);
  free(saucer);
  saucer = NULL;
}

void
saucer_shutdown(void)
{
  al_destroy_bitmap(small_sprite);
  al_destroy_bitmap(large_sprite);
}

void
saucer_update(SAUCER *saucer, SHIP *ship, ALLEGRO_TIMER *timer)
{
  saucer->position->x += saucer->velocity->x;
  saucer->position->y += saucer->velocity->y;
  wrap_position(saucer->position);

  if(!saucer->missile->active)
    saucer_fire(saucer, ship, timer);
  else
    update_missile(saucer->missile, timer);
}
