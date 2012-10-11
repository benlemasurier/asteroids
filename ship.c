#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
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
ship_drag(SHIP *ship)
{
  ship->velocity->x *= DRAG;
  ship->velocity->y *= DRAG;
}

bool
ship_explode(SHIP *ship, ALLEGRO_BITMAP **sprites, uint8_t n_frames)
{
  if(ship->explosion)
    return false;

  ANIMATION *explosion = new_animation(sprites, n_frames);

  // explosion->slowdown = 10;
  explosion->position->x = ship->position->x - (explosion->width  / 2);
  explosion->position->y = ship->position->y - (explosion->height / 2);

  ship->explosion = explosion;

  return true;
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

