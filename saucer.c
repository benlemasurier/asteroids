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
#include "level.h"
#include "ship.h"
#include "saucer.h"

#define SMALL_POINTS 1000
#define LARGE_POINTS 250 // FIXME: is this correct?

static ALLEGRO_BITMAP *small_sprite = NULL;
static ALLEGRO_BITMAP *large_sprite = NULL;

enum {
  TOP    = 0,
  RIGHT  = 1,
  BOTTOM = 2,
  LEFT   = 3
};

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

static VECTOR *
random_position(void)
{
  uint8_t entry;
  VECTOR *position = malloc(sizeof(VECTOR));

  /* FIXME: is this the correct way to get 0-3? */
  entry = (uint8_t) (rand() / (RAND_MAX / 4));
  switch(entry) {
    case TOP:
      position->x = rand_f(0, SCREEN_W);
      position->y = -10;
    case RIGHT:
      position->x = SCREEN_W + 10;
      position->y = rand_f(0, SCREEN_H);
    case BOTTOM:
      position->x = rand_f(0, SCREEN_W);
      position->y = SCREEN_H + 10;
    case LEFT:
      position->x = -10;
      position->y = rand_f(0, SCREEN_H);
  }

  return position;
}

SAUCER *
saucer_new(uint8_t size, uint8_t level)
{
  assert(small_sprite);
  assert(large_sprite);

  SAUCER *saucer      = malloc(sizeof(SAUCER));
  saucer->position    = random_position();
  saucer->velocity    = malloc(sizeof(VECTOR));
  saucer->angle       = rand_f(0.0, 360.0);
  saucer->level       = level;
  saucer->velocity->x = (float)   sin(deg2rad(saucer->angle));
  saucer->velocity->y = (float) -(cos(deg2rad(saucer->angle)));
  saucer->missile     = missile_create();

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
  float accuracy_offset;

  MISSILE *missile = saucer->missile;

  /* missile in use? */
  if(missile->active)
    return;

  missile->active = true;
  missile->position->x = saucer->position->x + (saucer->width  / 2);
  missile->position->y = saucer->position->y + (saucer->height / 2);
  missile->time = al_get_timer_count(timer);

  /* calculate the angle to hit the ship
   * my (current) best guess at calculating accuracy offsets
   *    f(x) = |i + (-n)|
   * where x = the accuracy offset angle,
   *       i = the current level
   *   and n = the highest possible level difficulty
  */
  accuracy_offset = abs(saucer->level + (-LEVEL_MAX));
  if(rand_f(0, 1) < 0.5)
    accuracy_offset = (-accuracy_offset);

  missile->angle = get_angle(ship->position->x, ship->position->y, missile->position->x, missile->position->y);
  missile->angle = rad2deg(missile->angle);
  missile->angle = missile->angle - 90.0;
  missile->angle += accuracy_offset;

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
    missile_update(saucer->missile, timer);
}
