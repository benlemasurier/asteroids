#ifndef asteroids_animation_H
#define asteroids_animation_H

#include "asteroids.h"

typedef struct animation_t {
  uint8_t width;
  uint8_t height;
  VECTOR *position;

  size_t  n_frames;
  size_t  current_frame;

  /* slowdown factor, play each frame `slowdown` times */
  uint8_t slowdown;

  /* how many times has the current frame been played? */
  uint8_t frame_played;

  ALLEGRO_BITMAP **sprites;
} ANIMATION;


void animation_draw(ANIMATION *animation);
void animation_free(ANIMATION *animation);
ANIMATION *animation_new(ALLEGRO_BITMAP **sprites, size_t n_frames);
void animation_update(ANIMATION *animation);

#endif /* asteroids_animation_H */
