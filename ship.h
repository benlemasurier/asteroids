#ifndef asteroids_ship_H
#define asteroids_ship_H

#include "asteroids.h"

#define ACCEL_SCALE   0.07

void ship_accelerate(SHIP *ship);
void ship_free(SHIP *ship);
void ship_rotate(SHIP *ship, float deg);

#endif /* asteroids_ship_H */
