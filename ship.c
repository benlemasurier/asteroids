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

void
ship_free(SHIP *ship)
{
  if(ship->explosion != NULL)
    free_animation(ship->explosion);

  free(ship->position);
  free(ship->velocity);
  free(ship);

  ship = NULL;
}

void
ship_rotate(SHIP *ship, float deg)
{
  ship->angle += deg;

  if(ship->angle > 360.0)
    ship->angle -= 360.0;
  if(ship->angle < 0)
    ship->angle += 360.0;
}

