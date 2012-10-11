#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#include "asteroids.h"
#include "animation.h"

void
animation_draw(ANIMATION *animation)
{
  if(animation->current_frame >= animation->n_frames)
    return;

  al_draw_bitmap(
      animation->sprites[animation->current_frame],
      animation->position->x,
      animation->position->y,
      DRAWING_FLAGS);
}

void
animation_free(ANIMATION *animation)
{
  free(animation->position);
  free(animation);
  animation = NULL;
}

ANIMATION *
animation_new(ALLEGRO_BITMAP **sprites, size_t n_frames)
{
  ANIMATION *animation = malloc(sizeof(ANIMATION));

  animation->width  = al_get_bitmap_width(sprites[0]);
  animation->height = al_get_bitmap_height(sprites[0]);
  animation->current_frame = 0;
  animation->frame_played  = 0;
  animation->slowdown      = 1;

  animation->position      = malloc(sizeof(VECTOR));
  animation->position->x   = 0;
  animation->position->y   = 0;

  animation->n_frames = n_frames;
  animation->sprites  = sprites;

  return animation;
}

void
animation_update(ANIMATION *animation)
{
  /* slow down animation playback by rendering
   * each frame multiple times */
  if(animation->frame_played < animation->slowdown) {
    animation->frame_played++;

    return;
  }

  animation->current_frame++;
  animation->frame_played = 0;
}
