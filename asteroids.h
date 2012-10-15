#ifndef asteroids_H
#define asteroids_H

#include <stdint.h>
#include <allegro5/allegro.h>

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

typedef struct list_t {
  void *data;
  struct list_t *prev;
  struct list_t *next;
} LIST;

void wrap_position(VECTOR *position);

LIST *list_append(LIST *list, void *data);
LIST *list_create(void);
LIST *list_first(LIST *list);
LIST *list_last(LIST *list);
LIST *list_remove(LIST *list, void *data);

#endif /* asteroids_H */
