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
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

const float FPS           = 60;
const int   SCREEN_W      = 800;
const int   SCREEN_H      = 600;
const float DRAG          = 0.98;
const float ACCEL_SCALE   = 0.2;
const float MISSILE_SPEED = 8;
const float MISSILE_TTL   = 1;
const int   MAX_MISSILES  = 4;
const int   START_LIVES   = 3;
const int   LIVES_X       = 50;
const int   LIVES_Y       = 50;
const int   HIGH_SCORE_Y  = 30;

enum CONTROLS {
  KEY_UP, KEY_LEFT, KEY_RIGHT, KEY_SPACE
};

struct vector {
  float x;
  float y;
};

struct ship {
  int width;
  int height;

  struct missile **missiles;

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
  int     lives;
  unsigned long int   score;
  struct  ship        *ship;

  ALLEGRO_DISPLAY     *display;
  ALLEGRO_TIMER       *timer;
  ALLEGRO_EVENT_QUEUE *event_queue;
  ALLEGRO_BITMAP      *lives_sprite;
  ALLEGRO_FONT        *font;
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

  al_init_font_addon();

  if(!al_init_ttf_addon()) {
    fprintf(stderr, "failed to initialize ttf system.\n");
    return false;
  }

  /* asteroids font */
  asteroids.font = al_load_ttf_font("data/vectorb.ttf", 12, 0);

  /* lives sprite */
  asteroids.lives_sprite = al_load_bitmap("data/sprites/ship.png");
  if(!asteroids.lives_sprite) {
    fprintf(stderr, "failed to load lives sprite.\n");
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

static struct missile *
create_missile(struct ship *ship)
{
  struct missile *missile = malloc(sizeof(struct missile));
  missile->position = malloc(sizeof(struct vector));
  missile->velocity = malloc(sizeof(struct vector));

  missile->sprite = al_load_bitmap("data/sprites/missile.png");
  missile->width  = al_get_bitmap_width(missile->sprite);
  missile->height = al_get_bitmap_height(missile->sprite);

  if(!missile->sprite) {
    free(missile->position);
    free(missile->velocity);
    free(missile);
    fprintf(stderr, "failed to create missile sprite.\n");

    return NULL;
  }

  missile->active = false;

  return missile;
}

static bool
create_ship(struct ship **ship)
{
  *ship = malloc(sizeof(struct ship));
  (*ship)->position = malloc(sizeof(struct vector));
  (*ship)->velocity = malloc(sizeof(struct vector));

  (*ship)->sprite = al_load_bitmap("data/sprites/ship.png");
  if(!(*ship)->sprite) {
    fprintf(stderr, "failed to create ship sprite.\n");
    return false;
  }

  (*ship)->width       = al_get_bitmap_width((*ship)->sprite);
  (*ship)->height      = al_get_bitmap_height((*ship)->sprite);
  (*ship)->angle       = 0.0;
  (*ship)->velocity->x = 0.0;
  (*ship)->velocity->y = 0.0;
  (*ship)->position->x = SCREEN_W / 2;
  (*ship)->position->y = SCREEN_H / 2;
  (*ship)->missiles = malloc(sizeof(struct misssile *) * MAX_MISSILES);

  int i;
  for(i = 0; i < MAX_MISSILES; i++)
    (*ship)->missiles[i] = create_missile((*ship));

  return true;
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
    asteroid->sprite = al_load_bitmap("data/sprites/asteroid-big.png");
  else if(asteroid->angle < 180.0)
    asteroid->sprite = al_load_bitmap("data/sprites/asteroid-big-90.png");
  else if(asteroid->angle < 270.0)
    asteroid->sprite = al_load_bitmap("data/sprites/asteroid-big-180.png");
  else if(asteroid->angle < 360.0)
    asteroid->sprite = al_load_bitmap("data/sprites/asteroid-big-270.png");

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

static void
launch_missile(struct ship *ship, struct missile *missile)
{
  missile->active = true;

  missile->angle  = ship->angle;

  missile->velocity->x = (float)   sin(deg2rad(ship->angle))  * MISSILE_SPEED;
  missile->velocity->y = (float) -(cos(deg2rad(ship->angle))) * MISSILE_SPEED;
  missile->position->x = ship->position->x;
  missile->position->y = ship->position->y;

  missile->time = al_get_timer_count(asteroids.timer);
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

static void
draw_high_score(void)
{
  char score[20];
  sprintf(score, "%02lu", asteroids.score);

  al_draw_text(asteroids.font,
      al_map_rgb(255,255,255),
      SCREEN_W / 2,
      SCORE_Y,
      ALLEGRO_ALIGN_CENTRE,
      score);
}

static void
draw_lives(void)
{
  int width = al_get_bitmap_width(asteroids.lives_sprite);

  for(int i = 0; i < asteroids.lives; i++)
    al_draw_bitmap(
        asteroids.lives_sprite,
        LIVES_X + (width * i),
        LIVES_Y,
        0);
}

static bool
collision(float b1_x, float b1_y, int b1_w, int b1_h,
          float b2_x, float b2_y, int b2_w, int b2_h)
{
  if((b1_x > b2_x + b2_w - 1) || /* is b1 on the right side of b2? */
     (b1_y > b2_y + b2_h - 1) || /* is b1 under b2?                */
     (b2_x > b1_x + b1_w - 1) || /* is b2 on the right side of b1? */
     (b2_y > b1_y + b1_h - 1))   /* is b2 under b1?                */
  {
    /* no collision */
    return false;
  }

  return true;
}

static bool
asteroid_collision(struct ship *ship, struct asteroid *asteroid)
{
  float ship_x = ship->position->x - (ship->width  / 2);
  float ship_y = ship->position->y - (ship->height / 2);
  float rock_x = asteroid->position->x - (asteroid->width  / 2);
  float rock_y = asteroid->position->y - (asteroid->height / 2);

  return collision(ship_x, ship_y, ship->width, ship->height,
                   rock_x, rock_y, asteroid->width, asteroid->height);
}

int
main(int argc, char **argv)
{
  asteroids.score       = 0;
  asteroids.lives       = START_LIVES;
  asteroids.display     = NULL;
  asteroids.timer       = NULL;
  asteroids.event_queue = NULL;

  bool key[4]   = { false, false, false, false };
  bool redraw   = true;
  bool quit     = false;
  bool debounce = false; /* fire debouce, force user to press for each fire */

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

      // shoot
      if(key[KEY_SPACE]) {
        int i;
        for(i = 0; i < MAX_MISSILES; i++) {
          if(!asteroids.ship->missiles[i]->active && !debounce) {
            launch_missile(asteroids.ship, asteroids.ship->missiles[i]);
            debounce = true;
            break;
          }
        }
      }

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
        case ALLEGRO_KEY_SPACE:
          key[KEY_SPACE] = true;
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
        case ALLEGRO_KEY_SPACE:
          key[KEY_SPACE] = false;
          debounce = false;
          break;
        case ALLEGRO_KEY_ESCAPE:
          quit = true;
          break;
      }
    }

    if(redraw && al_is_event_queue_empty(asteroids.event_queue)) {
      redraw = false;

      /* collisions */
      if(asteroid_collision(asteroids.ship, asteroid))
        printf("ZOMG COLLISION!!!\n");

      /* update positions */
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
      draw_high_core();
      draw_lives();

      int i;
      for(i = 0; i < MAX_MISSILES; i++) {
        if(asteroids.ship->missiles[i]->active) {
          if((asteroids.ship->missiles[i]->time + (MISSILE_TTL * FPS)) < al_get_timer_count(asteroids.timer)) {
            asteroids.ship->missiles[i]->active = false;
          } else {
            asteroids.ship->missiles[i]->position->x += asteroids.ship->missiles[i]->velocity->x;
            asteroids.ship->missiles[i]->position->y += asteroids.ship->missiles[i]->velocity->y;
            wrap_position(asteroids.ship->missiles[i]->position);

            draw_missile(asteroids.ship->missiles[i]);
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

  int i;
  for(i = 0; i < MAX_MISSILES; i++)
    free_missile(asteroids.ship->missiles[i]);
  free_asteroid(asteroid);
  free_ship(&asteroids.ship);

  exit(EXIT_SUCCESS);
}
