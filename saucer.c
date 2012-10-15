#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#include "saucer.h"

#define SMALL_POINTS 1000;
#define LARGE_POINTS 250;

static ALLEGRO_BITMAP *small_sprite = NULL;
static ALLEGRO_BITMAP *large_sprite = NULL;

bool
saucer_init(void)
{
  if((small_sprite = al_load_bitmap("data/sprites/saucer/saucer-small.png")) == NULL) {
    fprintf(stderr, "failed to load small saucer sprite\n");
    return false;
  }

  if((small_sprite = al_load_bitmap("data/sprites/saucer/saucer-small.png")) == NULL) {
    fprintf(stderr, "failed to load small saucer sprite\n");
    return false;
  }

  return true;
}

SAUCER *
saucer_new(uint8_t size)
{
  assert(small_sprite);
  assert(large_sprite);

  SAUCER *saucer = malloc(sizeof(SAUCER));
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

  return saucer;
}

void
saucer_free(SAUCER *saucer)
{
  free(saucer);
}

void
saucer_shutdown(void)
{
  al_destroy_bitmap(small_sprite);
  al_destroy_bitmap(large_sprite);
}
