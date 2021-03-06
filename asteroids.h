#ifndef asteroids_H
#define asteroids_H

#include <stdint.h>
#include <allegro5/allegro.h>

#define FPS  60
#define DRAG 0.99

/* original asteroids was 1024x760 */
#define FULLSCREEN false
#define SCREEN_W 1024
#define SCREEN_H 768
#define VOLUME 1.0

#define BLACK 0,0,0
#define WHITE 255,255,255

#define MISSILE_SPEED 8
#define MAX_MISSILES  4
#define START_LIVES   3
#define LIVES_X       84
#define LIVES_Y       60
#define SCORE_X       130
#define SCORE_Y       27
#define HIGH_SCORE_Y  30

#define DRAWING_FLAGS 0

enum CONTROLS {
  KEY_S,        /* start */
  KEY_UP,       /* thrust */
  KEY_LEFT,     /* rotate left */
  KEY_RIGHT,    /* rotate right */
  KEY_SPACE,    /* fire ze missiles */
  KEY_LCONTROL, /* HYPERSPACE! */
  KEY_ESCAPE    /* quit */
};

typedef struct vector_t {
  float x;
  float y;
} VECTOR;

bool collision(float b1_x, float b1_y, int b1_w, int b1_h, float b2_x, float b2_y, int b2_w, int b2_h);
void wrap_position(VECTOR *position);
bool offscreen(VECTOR *position, uint8_t width, uint8_t height);
int64_t seconds_elapsed(int64_t time_count);

#endif /* asteroids_H */
