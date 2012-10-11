#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "ship.h"
#include "util.h"
#include "asteroids.h"

void
ship_accelerate(SHIP *ship)
{
  ship->velocity->x += (float)   sin(deg2rad(ship->angle))  * ACCEL_SCALE;
  ship->velocity->y += (float) -(cos(deg2rad(ship->angle))) * ACCEL_SCALE;
}
