#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "asteroids.h"
#include "animation.h"
#include "explosion.h"

static ALLEGRO_BITMAP *sprites[15];

ANIMATION *
explosion_create(VECTOR *position)
{
  ANIMATION *explosion = animation_new(sprites, 15);

  explosion->slowdown = 2;
  explosion->position->x = position->x - (explosion->width  / 2);
  explosion->position->y = position->y - (explosion->height / 2);

  return explosion;
}

bool
explosion_init(void)
{
  for(int i = 0; i < 15; i++) {
    char name[255];
    sprintf(name, "data/sprites/asteroid/explosion/%d.png", i + 1);
    if((sprites[i] = al_load_bitmap(name)) == NULL)
      fprintf(stderr, "failed to load explosion sprite %d\n", i);
  }

  return true;
}
