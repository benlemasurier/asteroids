#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#include "list.h"
#include "util.h"
#include "missile.h"
#include "animation.h"
#include "asteroid.h"
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

bool
ship_asteroid_collision(SHIP *s, ASTEROID *a)
{
  float ship_x = s->position->x - (s->width  / 2);
  float ship_y = s->position->y - (s->height / 2);
  float rock_x = a->position->x - (a->width  / 2);
  float rock_y = a->position->y - (a->height / 2);

  return collision(ship_x, ship_y, s->width, s->height,
                   rock_x, rock_y, a->width, a->height);
}

SHIP *
ship_create(void)
{
  SHIP *ship     = malloc(sizeof(SHIP));
  ship->position = malloc(sizeof(VECTOR));
  ship->velocity = malloc(sizeof(VECTOR));
  ship->missiles = NULL;

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
  ship->missiles    = NULL;
  ship->thrust_visible = false;
  ship->fire_debounce  = false;

  for(int i = 0; i < MAX_MISSILES; i++)
    ship->missiles = list_append(ship->missiles, missile_create());

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

  /* explosion->slowdown = 10; */
  explosion->position->x = ship->position->x - (explosion->width  / 2);
  explosion->position->y = ship->position->y - (explosion->height / 2);

  ship->explosion = explosion;

  return true;
}

void
ship_fire(SHIP *ship, ALLEGRO_TIMER *timer)
{
  LIST *head = NULL;
  MISSILE *missile = NULL;
  VECTOR position;

  /* full button press required for each missile */
  if(ship->fire_debounce)
    return;

  /* find an inactive missile to launch */
  head = list_first(ship->missiles);
  while(head && !missile) {
    MISSILE *m = (MISSILE *) head->data;
    if(!m->active)
      missile = m;

    head = head->next;
  }

  /* all missiles in use? */
  if(missile == NULL)
    return;

  position.x = ship->position->x;
  position.y = ship->position->y;

  missile_fire(missile, &position, ship->angle, timer);
  ship->fire_debounce = true;
}

void
ship_free(SHIP *ship)
{
  LIST *head;

  if(ship->explosion)
    animation_free(ship->explosion);

  head = list_first(ship->missiles);
  while(head) {
    missile_free((MISSILE *) head->data);
    head = head->next;
  }

  /* FIXME: free missile list */

  free(ship->missiles);
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
  if((sprite = al_load_bitmap("data/sprites/ship/ship.png")) == NULL) {
    fprintf(stderr, "failed to load ship sprite\n");
    return false;
  }

  if((thrust_sprite = al_load_bitmap("data/sprites/ship/ship-thrust.png")) == NULL) {
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

SHIP *
ship_update(SHIP *ship, bool keys[], ALLEGRO_TIMER *timer)
{
  LIST *head = NULL;

  /* move forward */
  if(keys[KEY_UP])
    ship_accelerate(ship);

  /* rotate */
  if(keys[KEY_LEFT])
    ship_rotate(ship, -3);
  if(keys[KEY_RIGHT])
    ship_rotate(ship, 3);

  /* hyperspace */
  if(keys[KEY_LCONTROL])
    ship_hyperspace(ship);

  /* shoot */
  if(keys[KEY_SPACE])
    ship_fire(ship, timer);

  if(ship->explosion) {
    animation_update(ship->explosion);

    /* if the animation is complete, create a new ship */
    if(ship->explosion->current_frame >= ship->explosion->n_frames - 1) {
      /* FIXME: need preemptive collision detection, wait() */
      SHIP *old = ship;
      ship = ship_create();
      ship_free(old);
    }

    return ship;
  }

  /* ship missile positions */
  head = list_first(ship->missiles);
  while(head) {
    missile_update((MISSILE *) head->data, timer);
    head = head->next;
  }

  ship->position->x += ship->velocity->x;
  ship->position->y += ship->velocity->y;
  wrap_position(ship->position);

  /* slow down over time */
  ship_drag(ship);

  return ship;
}

