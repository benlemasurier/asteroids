#ifndef asteroids_asteroid_H
#define asteroids_asteroid_H

#define ASTEROID_LARGE  2
#define ASTEROID_MEDIUM 1
#define ASTEROID_SMALL  0

#define ASTEROID_LARGE_POINTS  20
#define ASTEROID_MEDIUM_POINTS 50
#define ASTEROID_SMALL_POINTS  100

#include "list.h"
#include "asteroids.h"

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

ASTEROID *asteroid_create(uint8_t size);
void asteroid_draw(ASTEROID *asteroid);
void asteroid_draw_list(LIST *rocks);
void asteroid_free(ASTEROID *asteroid);
bool asteroid_init(void);
void asteroid_shutdown(void);
void asteroid_update(ASTEROID *asteroid);
void asteroid_update_list(LIST *rocks);

#endif /* asteroids_asteroid_H */
