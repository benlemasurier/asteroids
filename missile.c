#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

#include "list.h"
#include "asteroid.h"
#include "asteroids.h"
#include "missile.h"

static ALLEGRO_BITMAP *sprite;
static ALLEGRO_SAMPLE *sample;

bool
missile_asteroid_collision(MISSILE *m, ASTEROID *a)
{
  if(!a)
    return false;

  float missile_x = m->position->x - (m->width  / 2);
  float missile_y = m->position->y - (m->height / 2);
  float rock_x = a->position->x - (a->width  / 2);
  float rock_y = a->position->y - (a->height / 2);

  return collision(missile_x, missile_y, m->width, m->height,
      rock_x, rock_y, a->width, a->height);
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
missile_draw_list(LIST *missiles)
{
  LIST *head = list_first(missiles);
  while(head) {
    MISSILE *m = (MISSILE *) head->data;

    if(m->active)
      missile_draw(m);

    head = head->next;
  }
}

void
missile_fire(MISSILE *missile, ALLEGRO_TIMER *timer)
{
  missile->active = true;
  missile->time = al_get_timer_count(timer);

  al_play_sample(missile->sample, 1.0, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
}

void
missile_free(MISSILE *missile)
{
  free(missile->position);
  free(missile->velocity);
  free(missile);

  missile = NULL;
}

bool
missile_init(void)
{
  /* preload sprite */
  if(!(sprite = al_load_bitmap("data/sprites/missile/missile.png"))) {
    fprintf(stderr, "unable to load missile sprite\n");
    return false;
  }

  if(!(sample = al_load_sample("data/sounds/missile.wav"))) {
    fprintf(stderr, "unable to load missile audio sample.\n");
    return false;
  }

  return true;
}

MISSILE *
missile_create(void)
{
  MISSILE *missile  = malloc(sizeof(MISSILE));
  missile->position = malloc(sizeof(VECTOR));
  missile->velocity = malloc(sizeof(VECTOR));

  missile->sample = sample;
  missile->sprite = sprite;
  missile->width  = al_get_bitmap_width(missile->sprite);
  missile->height = al_get_bitmap_height(missile->sprite);
  missile->angle  = 0;
  missile->active = false;

  if(!missile->sprite) {
    free(missile->position);
    free(missile->velocity);
    free(missile);
    fprintf(stderr, "failed to create missile sprite.\n");

    return NULL;
  }

  return missile;
}

void
missile_shutdown(void)
{
  al_destroy_bitmap(sprite);
  al_destroy_sample(sample);
}

void
missile_update(MISSILE *missile, ALLEGRO_TIMER *timer)
{
  if((missile->time + (MISSILE_TTL * FPS)) < al_get_timer_count(timer)) {
    missile->active = false;
    return;
  }

  missile->position->x += missile->velocity->x;
  missile->position->y += missile->velocity->y;
  wrap_position(missile->position);
}
