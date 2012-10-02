/*
 * asteroids
 * an asteroids clone
 *
 * ben lemasurier 2k12
 * https://github.com/benlemasurier/asteroids
 *
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

const float FPS         = 60;
const int   SCREEN_W    = 800;
const int   SCREEN_H    = 600;
const float DRAG        = 0.98;
const float ACCEL_SCALE = 0.2;

enum CONTROLS {
  KEY_UP, KEY_LEFT, KEY_RIGHT
};

struct vector {
  float x;
  float y;
};

struct ship {
  int width;
  int height;

  float angle;
  struct vector *position;
  struct vector *velocity;

  ALLEGRO_BITMAP *sprite;
};

struct asteroids {
  struct ship         *ship;
  ALLEGRO_DISPLAY     *display;
  ALLEGRO_TIMER       *timer;
  ALLEGRO_EVENT_QUEUE *event_queue;
} asteroids;

static void
shutdown()
{
  // FIXME: why can't I cleanly access asteroids.timer,display,etc here?
  printf("shutdown.\n");
}

void
rotate_ship(struct ship *ship, float deg)
{
  ship->angle += deg;

  if(ship->angle > 360.0)
    ship->angle -= 360.0;
  if(ship->angle < 0)
    ship->angle += 360.0;
}

void
wrap_position(struct vector *position)
{
  if(position->x > SCREEN_W)
    position->x = 0;
  if(position->x < 0)
    position->x = SCREEN_W;
  if(position->y > SCREEN_H)
    position->y = 0;
  if(position->y < 0)
    position->y = SCREEN_H;
}

float
deg2rad(float deg)
{
  return deg * (M_PI / 180);
}

static bool
init(void)
{
  if(!al_init()) {
    fprintf(stderr, "failed to initialize allegro.\n");
    return false;
  }

  if(!al_install_keyboard()) {
    fprintf(stderr, "failed to initialize keyboard.\n");
    return false;
  }

  if(!al_init_image_addon()) {
    fprintf(stderr, "failed to initialize image system.\n");
    return false;
  }

  asteroids.timer = al_create_timer(1.0 / FPS);
  if(!asteroids.timer) {
    fprintf(stderr, "failed to create timer.\n");
    return false;
  }

  asteroids.display = al_create_display(SCREEN_W, SCREEN_H);
  if(!asteroids.display) {
    fprintf(stderr, "failed to create display.\n");
    return false;
  }

  asteroids.event_queue = al_create_event_queue();
  if(!asteroids.event_queue) {
    fprintf(stderr, "failed to create event queue.\n");
    return false;
  }

  al_register_event_source(asteroids.event_queue, al_get_display_event_source(asteroids.display));
  al_register_event_source(asteroids.event_queue, al_get_timer_event_source(asteroids.timer));
  al_register_event_source(asteroids.event_queue, al_get_keyboard_event_source());

  return true;
}

static bool
create_ship(struct ship **ship)
{
  *ship = malloc(sizeof(struct ship));
  (*ship)->position = malloc(sizeof(struct vector));
  (*ship)->velocity = malloc(sizeof(struct vector));

  (*ship)->width       = 16;
  (*ship)->height      = 25;
  (*ship)->angle       = 0.0;
  (*ship)->velocity->x = 0.0;
  (*ship)->velocity->y = 0.0;
  (*ship)->position->x = SCREEN_W / 2;
  (*ship)->position->y = SCREEN_H / 2;

  (*ship)->sprite = al_load_bitmap("sprites/ship.png");
  if(!(*ship)->sprite) {
    fprintf(stderr, "failed to create ship sprite.\n");
    return false;
  }

  return true;
}

static void
accelerate(struct ship *ship)
{
  ship->velocity->x += (float)   sin(deg2rad(ship->angle))  * ACCEL_SCALE;
  ship->velocity->y += (float) -(cos(deg2rad(ship->angle))) * ACCEL_SCALE;
}

static void
drag(struct ship *ship)
{
  ship->velocity->x *= DRAG;
  ship->velocity->y *= DRAG;
}

static void
draw_ship(struct ship *ship)
{
  al_draw_rotated_bitmap(
      ship->sprite,
      ship->width  / 2,
      ship->height / 2,
      ship->position->x,
      ship->position->y,
      deg2rad(ship->angle), 0);
}

int
main(int argc, char **argv)
{
  asteroids.display     = NULL;
  asteroids.timer       = NULL;
  asteroids.event_queue = NULL;

  bool key[3] = { false, false, false };
  bool redraw = true;
  bool quit   = false;

  atexit(shutdown);

  // setup allegro engine
  if(!init())
    exit(EXIT_FAILURE);

  // this spacecraft must be built
  if(!create_ship(&asteroids.ship))
    exit(EXIT_FAILURE);

  al_flip_display();
  al_start_timer(asteroids.timer);

  while(!quit) {
    ALLEGRO_EVENT ev;
    al_wait_for_event(asteroids.event_queue, &ev);

    if(ev.type == ALLEGRO_EVENT_TIMER) {
      // move forward
      if(key[KEY_UP])
        accelerate(asteroids.ship);

      // rotate
      if(key[KEY_LEFT])
        rotate_ship(asteroids.ship, -3);
      if(key[KEY_RIGHT])
        rotate_ship(asteroids.ship, 3);

      redraw = true;
    } else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
      switch(ev.keyboard.keycode) {
        case ALLEGRO_KEY_UP:
          key[KEY_UP] = true;
          break;
        case ALLEGRO_KEY_LEFT:
          key[KEY_LEFT] = true;
          break;
        case ALLEGRO_KEY_RIGHT:
          key[KEY_RIGHT] = true;
          break;
      }
    } else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
      switch(ev.keyboard.keycode) {
        case ALLEGRO_KEY_UP:
          key[KEY_UP] = false;
          break;
        case ALLEGRO_KEY_LEFT:
          key[KEY_LEFT] = false;
          break;
        case ALLEGRO_KEY_RIGHT:
          key[KEY_RIGHT] = false;
          break;
        case ALLEGRO_KEY_ESCAPE:
          quit = true;
          break;
      }
    }

    if(redraw && al_is_event_queue_empty(asteroids.event_queue)) {
      redraw = false;

      asteroids.ship->position->x += asteroids.ship->velocity->x;
      asteroids.ship->position->y += asteroids.ship->velocity->y;

      // screen wrap
      wrap_position(asteroids.ship->position);

      al_clear_to_color(al_map_rgb(0, 0, 0));

      draw_ship(asteroids.ship);
      al_flip_display();

      // slow down over time
      drag(asteroids.ship);

      // debugging
      printf("ship = { x: %f, y: %f, angle: %f }\n",
          asteroids.ship->position->x, asteroids.ship->position->y, asteroids.ship->angle);
    }
  }

  if(asteroids.timer != NULL)
    al_destroy_timer(asteroids.timer);
  if(asteroids.event_queue != NULL)
    al_destroy_event_queue(asteroids.event_queue);
  if(asteroids.display != NULL)
    al_destroy_display(asteroids.display);

  if(asteroids.ship->position != NULL)
    free(asteroids.ship->position);
  if(asteroids.ship->velocity != NULL)
    free(asteroids.ship->velocity);

  if(asteroids.ship->sprite != NULL)
    al_destroy_bitmap(asteroids.ship->sprite);
  if(asteroids.ship != NULL)
    free(asteroids.ship);

  exit(EXIT_SUCCESS);
}
