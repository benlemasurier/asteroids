#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "ship.h"
#include "util.h"
#include "animation.h"
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

  ANIMATION *explosion = animation_new(sprites, n_frames);

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
    animation_free(ship->explosion);

  free(ship->position);
  free(ship->velocity);
  free(ship);

  ship = NULL;
}

void
ship_hyperspace(SHIP *ship)
{
  if(ship->hyper_debounce)
    return;

  ship->hyper_debounce = true;
  ship->velocity->x = 0.0;
  ship->velocity->y = 0.0;
  ship->position->x = rand_f(0, SCREEN_W);
  ship->position->y = rand_f(0, SCREEN_H);
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

void
ship_draw(SHIP *ship, bool thrusting)
{
  if(ship->explosion != NULL) {
    animation_draw(ship->explosion);
    return;
  }

  ALLEGRO_BITMAP *sprite;

  /* this creates a flashing thrust visualization
   * not _exactly_ like the original (too fast), but close. (FIXME) */
  sprite = ship->sprite;
  if(thrusting && !ship->thrust_visible)
    sprite = ship->thrust_sprite;

  al_draw_rotated_bitmap(
      sprite,
      ship->width  / 2,
      ship->height / 2,
      ship->position->x,
      ship->position->y,
      deg2rad(ship->angle),
      DRAWING_FLAGS);

  ship->thrust_visible = (ship->thrust_visible) ? false : true;
}

void
ship_update(SHIP *ship)
{
  if(ship->explosion) {
    animation_update(ship->explosion);

    /* if the animation is complete, create a new ship */
    if(ship->explosion->current_frame >= ship->explosion->n_frames) {
      ship_free(ship);
      /* FIXME: need preemptive collision detection, wait() */
      ship = create_ship();
    }

    return;
  }

  ship->position->x += ship->velocity->x;
  ship->position->y += ship->velocity->y;
  wrap_position(ship->position);

  /* slow down over time */
  ship_drag(ship);
}

