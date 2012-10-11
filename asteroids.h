#ifndef asteroids_H
#define asteroids_H

#include <stdio.h>
#include <stdint.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#define DRAG    0.99

#define SCREEN_W      800
#define SCREEN_H      600

#define ASTEROID_LARGE  2
#define ASTEROID_MEDIUM 1
#define ASTEROID_SMALL  0

#define ASTEROID_LARGE_POINTS  20
#define ASTEROID_MEDIUM_POINTS 50
#define ASTEROID_SMALL_POINTS  100

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
SHIP *create_ship(void);
bool preload_asteroid_sprites(void);
ALLEGRO_BITMAP *load_asteroid_sprite(uint8_t size, float angle);

#endif /* asteroids_H */
