#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#include "util.h"
#include "missile.h"
#include "animation.h"
#include "asteroids.h"
#include "ship.h"

static ALLEGRO_BITMAP *sprite;
static ALLEGRO_BITMAP *thrust_sprite;
static ALLEGRO_BITMAP *explosion_sprites[60];

void
ship_accelerate(SHIP *ship)
{
  ship->velocity->x += (float)   sin(deg2rad(ship->angle))  * ACCEL_SCALE;
  ship->velocity->y += (float) -(cos(deg2rad(ship->angle))) * ACCEL_SCALE;
}

SHIP *
ship_create(void)
{
  SHIP *ship = malloc(sizeof(SHIP));
  ship->position = malloc(sizeof(VECTOR));
  ship->velocity = malloc(sizeof(VECTOR));

  ship->sprite = sprite;
  ship->thrust_sprite = thrust_sprite;

  ship->width       = al_get_bitmap_width(ship->sprite);
  ship->height      = al_get_bitmap_height(ship->sprite);
  ship->angle       = 0.0;
  ship->velocity->x = 0.0;
  ship->velocity->y = 0.0;
  ship->position->x = SCREEN_W / 2;
  ship->position->y = SCREEN_H / 2;
  ship->explosion   = NULL;
  ship->missiles = malloc(sizeof(MISSILE *) * MAX_MISSILES);
  ship->thrust_visible = false;
  ship->fire_debounce  = false;

  for(int i = 0; i < MAX_MISSILES; i++)
    ship->missiles[i] = create_missile();

  return ship;
}

void
ship_drag(SHIP *ship)
{
  ship->velocity->x *= DRAG;
  ship->velocity->y *= DRAG;
}

bool
ship_explode(SHIP *ship)
{
  if(ship->explosion)
    return false;

  ANIMATION *explosion = animation_new(explosion_sprites, 60);

  // explosion->slowdown = 10;
  explosion->position->x = ship->position->x - (explosion->width  / 2);
  explosion->position->y = ship->position->y - (explosion->height / 2);

  ship->explosion = explosion;

  return true;
}

void
ship_fire(SHIP *ship, ALLEGRO_TIMER *timer)
{
  MISSILE *missile = NULL;

  /* full button press required for each missile */
  if(ship->fire_debounce)
    return;

  /* find an inactive missile to launch */
  for(int i = 0; i < MAX_MISSILES && missile == NULL; i++)
    if(!ship->missiles[i]->active)
      missile = ship->missiles[i];

  /* all missiles in use? */
  if(missile == NULL)
    return;

  ship->fire_debounce = true;

  missile->active = true;
  missile->angle  = ship->angle;
  missile->velocity->x = (float)   sin(deg2rad(ship->angle))  * MISSILE_SPEED;
  missile->velocity->y = (float) -(cos(deg2rad(ship->angle))) * MISSILE_SPEED;
  missile->position->x = ship->position->x;
  missile->position->y = ship->position->y;

  missile->time = al_get_timer_count(timer);
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

bool
ship_init(void)
{
  if((sprite = al_load_bitmap("data/sprites/ship.png")) == NULL) {
    fprintf(stderr, "failed to load ship sprite\n");
    return false;
  }

  if((thrust_sprite = al_load_bitmap("data/sprites/ship-thrust.png")) == NULL) {
    fprintf(stderr, "failed to load ship thrust sprite\n");
    return false;
  }

  /* ship explosion animation frames */
  for(int i = 0; i < 60; i++) {
    char name[255];
    sprintf(name, "data/sprites/ship/explosion/%d.png", i + 1);
    if((explosion_sprites[i] = al_load_bitmap(name)) == NULL) {
      fprintf(stderr, "failed to load ship explosion sprite %d\n", i);
      return false;
    }
  }

  return true;
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

  ALLEGRO_BITMAP *current_sprite;

  /* this creates a flashing thrust visualization
   * not _exactly_ like the original (too fast), but close. (FIXME) */
  current_sprite = ship->sprite;
  if(thrusting && !ship->thrust_visible)
    current_sprite = ship->thrust_sprite;

  al_draw_rotated_bitmap(
      current_sprite,
      ship->width  / 2,
      ship->height / 2,
      ship->position->x,
      ship->position->y,
      deg2rad(ship->angle),
      DRAWING_FLAGS);

  ship->thrust_visible = (ship->thrust_visible) ? false : true;
}

void
ship_shutdown(void)
{
  al_destroy_bitmap(sprite);
  al_destroy_bitmap(thrust_sprite);
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
      ship = ship_create();
    }

    return;
  }

  ship->position->x += ship->velocity->x;
  ship->position->y += ship->velocity->y;
  wrap_position(ship->position);

  /* slow down over time */
  ship_drag(ship);
}

