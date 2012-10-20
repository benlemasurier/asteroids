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
#include "asteroid.h"
#include "missile.h"
#include "level.h"
#include "ship.h"
#include "saucer.h"

#define SMALL_POINTS 1000
#define LARGE_POINTS 200

static ALLEGRO_BITMAP *small_sprite = NULL;
static ALLEGRO_BITMAP *large_sprite = NULL;

enum {
  TOP    = 0,
  RIGHT  = 1,
  BOTTOM = 2,
  LEFT   = 3
};

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

bool
saucer_asteroid_collision(SAUCER *s, ASTEROID *a)
{
  float saucer_x = s->position->x - (s->width  / 2);
  float saucer_y = s->position->y - (s->height / 2);
  float rock_x = a->position->x - (a->width  / 2);
  float rock_y = a->position->y - (a->height / 2);

  return collision(saucer_x, saucer_y, s->width, s->height,
                   rock_x, rock_y, a->width, a->height);
}

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

void
saucer_exit(SAUCER *saucer)
{
  /* pick a random exit point */
  saucer->exit = random_position();

  /* change saucer trajectory to exit point */
  saucer->angle = get_angle(saucer->exit->x, saucer->exit->y,
                            saucer->position->x, saucer->position->y);
  saucer->angle = rad2deg(saucer->angle);
  saucer->angle = saucer->angle - 90.0;

  saucer->velocity->x = (float)   sin(deg2rad(saucer->angle));
  saucer->velocity->y = (float) -(cos(deg2rad(saucer->angle)));
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
saucer_new(uint8_t size, uint8_t level, ALLEGRO_TIMER *timer)
{
  assert(small_sprite);
  assert(large_sprite);

  SAUCER *saucer      = malloc(sizeof(SAUCER));
  saucer->entry_time  = al_get_timer_count(timer);
  saucer->position    = random_position();
  saucer->velocity    = malloc(sizeof(VECTOR));
  saucer->exit        = NULL;
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

  missile->position->x = saucer->position->x + (saucer->width  / 2);
  missile->position->y = saucer->position->y + (saucer->height / 2);

  /* calculate the angle to hit the ship
   * my (current) best guess at calculating accuracy offsets
   *    f(x) = |i + (-n)| +/- rand(x)
   * where x = the accuracy offset angle,
   *       i = the current level
   *   and n = the highest possible level difficulty
  */
  accuracy_offset = abs(saucer->level + (-LEVEL_MAX));
  accuracy_offset = accuracy_offset + rand_f(-accuracy_offset, accuracy_offset);
  if(rand_f(0, 1) < 0.5)
    accuracy_offset = (-accuracy_offset);

  missile->angle = get_angle(ship->position->x, ship->position->y, missile->position->x, missile->position->y);
  missile->angle = rad2deg(missile->angle);
  missile->angle = missile->angle - 90.0;
  missile->angle += accuracy_offset;

  missile->velocity->x = (float)   sin(deg2rad(missile->angle))  * MISSILE_SPEED;
  missile->velocity->y = (float) -(cos(deg2rad(missile->angle))) * MISSILE_SPEED;

  missile_fire(missile, timer);
}

void
saucer_free(SAUCER *saucer)
{
  if(saucer->exit)
    free(saucer->exit);
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

SAUCER *
saucer_update(SAUCER *saucer, SHIP *ship, ALLEGRO_TIMER *timer)
{
  int64_t time_count = al_get_timer_count(timer);

  if(!saucer->exit)
    if(seconds_elapsed(time_count - saucer->entry_time) > SAUCER_TTL)
      saucer_exit(saucer);

  saucer->position->x += saucer->velocity->x;
  saucer->position->y += saucer->velocity->y;

  if(!saucer->missile->active)
    saucer_fire(saucer, ship, timer);
  else
    missile_update(saucer->missile, timer);

  /* if we're exiting and gone offscreen, remove the saucer */
  if(!saucer->exit) {
    wrap_position(saucer->position);
  } else {
    if(offscreen(saucer->position, saucer->width, saucer->height)) {
      saucer_free(saucer);
      saucer = NULL;
    }
  }

  return saucer;
}
