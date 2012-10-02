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
#include <time.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

const float FPS           = 60;
const int   SCREEN_W      = 800;
const int   SCREEN_H      = 600;
const float DRAG          = 0.98;
const float ACCEL_SCALE   = 0.2;
const float MISSILE_SPEED = 5;
const float MISSILE_TTL   = 1.3;

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

struct missile {
  int width;
  int height;
  bool active;

  float angle;
  struct vector *position;
  struct vector *velocity;

  int64_t time;

  ALLEGRO_BITMAP *sprite;
};

struct asteroid {
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

float
rand_f(float low, float high)
{
  srand((unsigned) time(0));
  return low + (float) rand() / ((float) RAND_MAX / (high - low));
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

  al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST);
  al_set_new_display_option(ALLEGRO_SAMPLES,        8, ALLEGRO_SUGGEST);
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
free_ship(struct ship **ship)
{
  if((*ship)->position != NULL)
    free((*ship)->position);
  if((*ship)->velocity != NULL)
    free((*ship)->velocity);

  if((*ship)->sprite != NULL)
    al_destroy_bitmap((*ship)->sprite);
  if((*ship) != NULL)
    free((*ship));

  ship = NULL;
}

static void
free_asteroid(struct asteroid *asteroid)
{
  if(asteroid->position != NULL)
    free(asteroid->position);
  if(asteroid->velocity != NULL)
    free(asteroid->velocity);

  if(asteroid->sprite != NULL)
    al_destroy_bitmap(asteroid->sprite);
  if(asteroid != NULL)
    free(asteroid);

  asteroid = NULL;
}

static void
free_missile(struct missile *missile)
{
  if(missile == NULL)
    return;

  if(missile->position != NULL)
    free(missile->position);
  if(missile->velocity != NULL)
    free(missile->velocity);

  if(missile->sprite != NULL) {
    al_destroy_bitmap(missile->sprite);
    missile->sprite = NULL;
  }

  if(missile != NULL)
    free(missile);

  missile = NULL;
}

static struct asteroid *
create_asteroid(void)
{
  struct asteroid *asteroid = malloc(sizeof(struct asteroid));
  asteroid->position = malloc(sizeof(struct vector));
  asteroid->velocity = malloc(sizeof(struct vector));

  /* what the fuck?
   *
   * rotated bitmaps are horribly pixelated
   * this is a hack to get nice, clean sprites */
  asteroid->angle  = rand_f(0.0, 360.0);
  if(asteroid->angle < 90.0)
    asteroid->sprite = al_load_bitmap("sprites/asteroid-big.png");
  else if(asteroid->angle < 180.0)
    asteroid->sprite = al_load_bitmap("sprites/asteroid-big-90.png");
  else if(asteroid->angle < 270.0)
    asteroid->sprite = al_load_bitmap("sprites/asteroid-big-180.png");
  else if(asteroid->angle < 360.0)
    asteroid->sprite = al_load_bitmap("sprites/asteroid-big-270.png");

  if(!asteroid->sprite) {
    free(asteroid->position);
    free(asteroid->velocity);
    free(asteroid);
    fprintf(stderr, "failed to create asteroid sprite.\n");

    return NULL;
  }

  asteroid->width  = al_get_bitmap_width(asteroid->sprite);
  asteroid->height = al_get_bitmap_height(asteroid->sprite);

  asteroid->velocity->x = rand_f(0.3, 1.2);
  asteroid->velocity->y = rand_f(0.3, 1.2);
  asteroid->position->x = rand_f(1.0, SCREEN_W);
  asteroid->position->y = rand_f(1.0, SCREEN_H);

  return asteroid;
}

static struct missile *
launch_missile(struct ship *ship)
{
  struct missile *missile = malloc(sizeof(struct missile));
  missile->position = malloc(sizeof(struct vector));
  missile->velocity = malloc(sizeof(struct vector));

  missile->sprite = al_load_bitmap("sprites/missile.png");

  if(!missile->sprite) {
    free(missile->position);
    free(missile->velocity);
    free(missile);
    fprintf(stderr, "failed to create missile sprite.\n");

    return NULL;
  }

  missile->active = true;
  missile->angle  = ship->angle;
  missile->width  = al_get_bitmap_width(missile->sprite);
  missile->height = al_get_bitmap_height(missile->sprite);

  missile->velocity->x = (float)   sin(deg2rad(ship->angle))  * MISSILE_SPEED;
  missile->velocity->y = (float) -(cos(deg2rad(ship->angle))) * MISSILE_SPEED;
  missile->position->x = ship->position->x;
  missile->position->y = ship->position->y;

  missile->time = al_get_timer_count(asteroids.timer);

  return missile;
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

static void
draw_asteroid(struct asteroid *asteroid)
{
  al_draw_bitmap(
      asteroid->sprite,
      asteroid->position->x - (asteroid->width  / 2),
      asteroid->position->y - (asteroid->height / 2),
      0);
}

static void
draw_missile(struct missile *missile)
{
  al_draw_bitmap(
      missile->sprite,
      missile->position->x - (missile->width  / 2),
      missile->position->y - (missile->height / 2),
      0);
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

  struct asteroid *asteroid;

  atexit(shutdown);

  // setup allegro engine
  if(!init())
    exit(EXIT_FAILURE);

  // this spacecraft must be built
  if(!create_ship(&asteroids.ship))
    exit(EXIT_FAILURE);

  // create an asteroid
  if((asteroid = create_asteroid()) == NULL)
    exit(EXIT_FAILURE);

  al_flip_display();
  al_start_timer(asteroids.timer);

  bool test = false;
  struct missile *missile = NULL;
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

      if(!test) {
        missile = launch_missile(asteroids.ship);
        test = true;
      }

      asteroids.ship->position->x += asteroids.ship->velocity->x;
      asteroids.ship->position->y += asteroids.ship->velocity->y;

      asteroid->position->x += asteroid->velocity->x;
      asteroid->position->y += asteroid->velocity->y;

      // screen wrap
      wrap_position(asteroids.ship->position);
      wrap_position(asteroid->position);

      al_clear_to_color(al_map_rgb(0, 0, 0));

      draw_ship(asteroids.ship);
      draw_asteroid(asteroid);

      // FIXME: unfuck.
      if(missile != NULL) {
        if(missile->active) {
          if((missile->time + (MISSILE_TTL * FPS)) < al_get_timer_count(asteroids.timer)) {
            missile->active = false;
          } else {
            missile->position->x += missile->velocity->x;
            missile->position->y += missile->velocity->y;
            wrap_position(missile->position);

            draw_missile(missile);
          }
        }
      }

      al_flip_display();

      // slow down over time
      drag(asteroids.ship);
    }
  }

  if(asteroids.timer != NULL)
    al_destroy_timer(asteroids.timer);
  if(asteroids.event_queue != NULL)
    al_destroy_event_queue(asteroids.event_queue);
  if(asteroids.display != NULL)
    al_destroy_display(asteroids.display);

  free_missile(missile);
  free_asteroid(asteroid);
  free_ship(&asteroids.ship);

  exit(EXIT_SUCCESS);
}
