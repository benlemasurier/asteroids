#ifndef asteroids_ship_H
#define asteroids_ship_H

#include "missile.h"
#include "animation.h"
#include "asteroids.h"

#define ACCEL_SCALE   0.07

typedef struct ship_t {
  int width;
  int height;
  bool thrust_visible;
  bool fire_debounce;
  bool hyper_debounce;

  LIST *missiles;

  float angle;
  VECTOR *position;
  VECTOR *velocity;

  ANIMATION *explosion;

  ALLEGRO_BITMAP *sprite;
  ALLEGRO_BITMAP *thrust_sprite;
} SHIP;

void ship_accelerate(SHIP *ship);
SHIP *ship_create(void);
void ship_drag(SHIP *ship);
void ship_draw(SHIP *ship, bool thrusting);
bool ship_explode(SHIP *ship);
void ship_fire(SHIP *ship, ALLEGRO_TIMER *timer);
void ship_free(SHIP *ship);
void ship_hyperspace(SHIP *ship);
bool ship_init(void);
void ship_rotate(SHIP *ship, float deg);
void ship_shutdown(void);
SHIP *ship_update(SHIP *ship, ALLEGRO_TIMER *timer);

#endif /* asteroids_ship_H */
