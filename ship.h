#ifndef asteroids_ship_H
#define asteroids_ship_H

#include "asteroids.h"

#define ACCEL_SCALE   0.07

void ship_accelerate(SHIP *ship);
SHIP *ship_create(void);
void ship_drag(SHIP *ship);
bool ship_explode(SHIP *ship);
void ship_fire(SHIP *ship, ALLEGRO_TIMER *timer);
void ship_free(SHIP *ship);
void ship_hyperspace(SHIP *ship);
bool ship_init(void);
void ship_rotate(SHIP *ship, float deg);
void ship_draw(SHIP *ship, bool thrusting);
void ship_shutdown(void);
void ship_update(SHIP *ship);

#endif /* asteroids_ship_H */
