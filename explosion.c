#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "asteroids.h"
#include "animation.h"
#include "explosion.h"

static uint8_t   n_explosions;
static ANIMATION **explosions;
static ALLEGRO_BITMAP *sprites[15];

void
new_explosion(VECTOR *position)
{
  ANIMATION *explosion = animation_new(sprites, 15);

  explosion->slowdown = 2;
  explosion->position->x = position->x - (explosion->width  / 2);
  explosion->position->y = position->y - (explosion->height / 2);

  n_explosions++;
  explosions = (ANIMATION **) realloc(explosions, sizeof(ANIMATION *) * n_explosions);
  explosions[n_explosions - 1] = explosion;
}

void
explosions_draw(void)
{
  for(int i = 0; i < n_explosions; i++)
    animation_draw(explosions[i]);
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

void
explosions_update(void)
{
  for(int i = 0; i < n_explosions; i++)
    if(explosions[i]->current_frame < explosions[i]->n_frames)
      animation_update(explosions[i]);
    else
      remove_explosion(explosions[i]);
}

void
remove_explosion(ANIMATION *explosion)
{
  ANIMATION **temp = malloc(sizeof(ANIMATION *) * n_explosions - 1);
  for(int i = 0, j = 0; i < n_explosions; i++) {
    if(explosions[i] != explosion) {
      temp[j] = explosions[i];
      j++;
    }
  }

  free(explosions);
  explosions = temp;
  n_explosions--;

  animation_free(explosion);
}
