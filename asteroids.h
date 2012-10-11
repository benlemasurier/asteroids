#ifndef asteroids_H
#define asteroids_H

#include <stdio.h>
#include <stdint.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#define FPS  60
#define DRAG 0.99

#define SCREEN_W 800
#define SCREEN_H 600

#define MISSILE_SPEED 8
#define MAX_MISSILES  4
#define START_LIVES   3
#define LIVES_X       84
#define LIVES_Y       60
#define SCORE_X       130
#define SCORE_Y       27
#define HIGH_SCORE_Y  30

#define DRAWING_FLAGS 0

typedef struct vector_t {
  float x;
  float y;
} VECTOR;

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

typedef struct missile_t {
  int width;
  int height;
  bool active;
  int64_t time;

  float angle;
  VECTOR *position;
  VECTOR *velocity;

  ALLEGRO_BITMAP *sprite;
} MISSILE;

typedef struct ship_t {
  int width;
  int height;
  bool thrust_visible;
  bool fire_debounce;
  bool hyper_debounce;

  MISSILE **missiles;

  float angle;
  VECTOR *position;
  VECTOR *velocity;

  ANIMATION *explosion;

  ALLEGRO_BITMAP *sprite;
  ALLEGRO_BITMAP *thrust_sprite;
} SHIP;

typedef struct asteroid_t {
  int width;
  int height;
  float angle;
  uint8_t size;
  uint8_t points;

  VECTOR *position;
  VECTOR *velocity;

  ALLEGRO_BITMAP *sprite;
} ASTEROID;

void wrap_position(VECTOR *position);

#endif /* asteroids_H */
