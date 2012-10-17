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

enum {
  LARGE       = 0,
  LARGE_90    = 1,
  LARGE_180   = 2,
  LARGE_270   = 3,
  MEDIUM      = 4,
  MEDIUM_90   = 5,
  MEDIUM_180  = 6,
  MEDIUM_270  = 7,
  SMALL       = 8,
  SMALL_90    = 9,
  SMALL_180   = 10,
  SMALL_270   = 11
};

static ALLEGRO_BITMAP *sprites[12];

static ALLEGRO_BITMAP *
load_asteroid_sprite(uint8_t size, float angle)
{
  ALLEGRO_BITMAP *sprite = NULL;

  /* what the fuck?
   *
   * rotated bitmaps are horribly pixelated
   * this is a hack to get nice, clean sprites
   *
   * also, this is _terribly_ messy. FIXME */
  switch(size) {
    case ASTEROID_LARGE:
      if(angle < 90.0)
        sprite = sprites[LARGE];
      else if(angle < 180.0)
        sprite = sprites[LARGE_90];
      else if(angle < 270.0)
        sprite = sprites[LARGE_180];
      else if(angle < 360.0)
        sprite = sprites[LARGE_270];
      break;
    case ASTEROID_MEDIUM:
      if(angle < 90.0)
        sprite = sprites[MEDIUM];
      else if(angle < 180.0)
        sprite = sprites[MEDIUM_90];
      else if(angle < 270.0)
        sprite = sprites[MEDIUM_180];
      else if(angle < 360.0)
        sprite = sprites[MEDIUM_270];
      break;
    case ASTEROID_SMALL:
      if(angle < 90.0)
        sprite = sprites[SMALL];
      else if(angle < 180.0)
        sprite = sprites[SMALL_90];
      else if(angle < 270.0)
        sprite = sprites[SMALL_180];
      else if(angle < 360.0)
        sprite = sprites[SMALL_270];
      break;
  }

  if(!sprite) {
    fprintf(stderr, "failed to load asteroid sprite.\n");
    return NULL;
  }

  return sprite;
}

ASTEROID *
asteroid_create(uint8_t size)
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
asteroid_free(ASTEROID *asteroid)
{
  free(asteroid->position);
  free(asteroid->velocity);
  free(asteroid);

  asteroid = NULL;
}

bool
asteroid_init(void)
{
  /* FIXME: fugtf */
  if((sprites[LARGE] = al_load_bitmap("data/sprites/asteroid/large/default.png")) == NULL)
    fprintf(stderr, "failed to load large/default.png\n");
  if((sprites[LARGE_90] = al_load_bitmap("data/sprites/asteroid/large/90.png")) == NULL)
    fprintf(stderr, "failed to load large/90.png\n");
  if((sprites[LARGE_180] = al_load_bitmap("data/sprites/asteroid/large/180.png")) == NULL)
    fprintf(stderr, "failed to load large/180.png\n");
  if((sprites[LARGE_270] = al_load_bitmap("data/sprites/asteroid/large/270.png")) == NULL)
    fprintf(stderr, "failed to load large/270.png\n");
  if((sprites[MEDIUM] = al_load_bitmap("data/sprites/asteroid/medium/default.png")) == NULL)
    fprintf(stderr, "failed to load medium/default.png\n");
  if((sprites[MEDIUM_90] = al_load_bitmap("data/sprites/asteroid/medium/90.png")) == NULL)
    fprintf(stderr, "failed to load medium/90.png\n");
  if((sprites[MEDIUM_180] = al_load_bitmap("data/sprites/asteroid/medium/180.png")) == NULL)
    fprintf(stderr, "failed to load medium/180.png\n");
  if((sprites[MEDIUM_270] = al_load_bitmap("data/sprites/asteroid/medium/270.png")) == NULL)
    fprintf(stderr, "failed to load medium/270.png\n");
  if((sprites[SMALL] = al_load_bitmap("data/sprites/asteroid/small/default.png")) == NULL)
    fprintf(stderr, "failed to load small/default.png\n");
  if((sprites[SMALL_90] = al_load_bitmap("data/sprites/asteroid/small/90.png")) == NULL)
    fprintf(stderr, "failed to load small/90.png\n");
  if((sprites[SMALL_180] = al_load_bitmap("data/sprites/asteroid/small/180.png")) == NULL)
    fprintf(stderr, "failed to load small/180.png\n");
  if((sprites[SMALL_270] = al_load_bitmap("data/sprites/asteroid/small/270.png")) == NULL)
    fprintf(stderr, "failed to load small/270.png\n");

  return true;
}

void
asteroid_shutdown(void)
{
  al_destroy_bitmap(sprites[LARGE]);
  al_destroy_bitmap(sprites[LARGE_90]);
  al_destroy_bitmap(sprites[LARGE_180]);
  al_destroy_bitmap(sprites[LARGE_270]);
  al_destroy_bitmap(sprites[MEDIUM]);
  al_destroy_bitmap(sprites[MEDIUM_90]);
  al_destroy_bitmap(sprites[MEDIUM_180]);
  al_destroy_bitmap(sprites[MEDIUM_270]);
  al_destroy_bitmap(sprites[SMALL]);
  al_destroy_bitmap(sprites[SMALL_90]);
  al_destroy_bitmap(sprites[SMALL_180]);
  al_destroy_bitmap(sprites[SMALL_270]);
}

void
asteroid_update(ASTEROID *asteroid)
{
  asteroid->position->x += asteroid->velocity->x;
  asteroid->position->y += asteroid->velocity->y;
  wrap_position(asteroid->position);
}
