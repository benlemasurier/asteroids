#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

#include "asteroids.h"
#include "animation.h"
#include "explosion.h"

static ALLEGRO_BITMAP *sprites[15];
static ALLEGRO_SAMPLE *sample;

ANIMATION *
explosion_create(VECTOR *position)
{
  ANIMATION *explosion = animation_new(sprites, 15);

  explosion->slowdown = 2;
  explosion->position->x = position->x - (explosion->width  / 2);
  explosion->position->y = position->y - (explosion->height / 2);

  al_play_sample(sample, VOLUME, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);

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
  
  if(!(sample = al_load_sample("data/sounds/explosion.wav"))) {
    fprintf(stderr, "unable to load explosion audio sample.\n");
    return false;
  }

  return true;
}

void
explosion_shutdown(void)
{
  al_destroy_sample(sample);
}
