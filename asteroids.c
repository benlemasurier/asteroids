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
#include <sys/time.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#define ASTEROID_LARGE  0
#define ASTEROID_MEDIUM 1
#define ASTEROID_SMALL  2

#define ASTEROID_LARGE_POINTS  20
#define ASTEROID_MEDIUM_POINTS 50
#define ASTEROID_SMALL_POINTS  100

const float FPS           = 60;
const int   SCREEN_W      = 800;
const int   SCREEN_H      = 600;
const float DRAG          = 0.99;
const float ACCEL_SCALE   = 0.07;
const float MISSILE_SPEED = 8;
const float MISSILE_TTL   = 1;
const int   MAX_MISSILES  = 4;
const int   START_LIVES   = 3;
const int   LIVES_X       = 84;
const int   LIVES_Y       = 60;
const int   SCORE_X       = 130;
const int   SCORE_Y       = 27;
const int   HIGH_SCORE_Y  = 30;

enum CONTROLS {
  KEY_UP, KEY_LEFT, KEY_RIGHT, KEY_SPACE
};

struct vector {
  float x;
  float y;
};

struct level {
  int n_asteroids;
  struct asteroid **asteroids;
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

struct ship {
  int width;
  int height;
  bool thrust_visible;

  struct missile **missiles;

  float angle;
  struct vector *position;
  struct vector *velocity;

  ALLEGRO_BITMAP *sprite;
  ALLEGRO_BITMAP *thrust_sprite;
};

struct asteroid {
  int width;
  int height;
  uint8_t size;
  uint8_t points;

  float angle;
  struct vector *position;
  struct vector *velocity;

  ALLEGRO_BITMAP *sprite;
};

struct asteroids {
  int     lives;
  unsigned long int   score;
  unsigned long int   high_score;
  struct ship         *ship;
  struct level        *level;

  ALLEGRO_DISPLAY     *display;
  ALLEGRO_TIMER       *timer;
  ALLEGRO_EVENT_QUEUE *event_queue;
  ALLEGRO_FONT        *small_font;
  ALLEGRO_FONT        *large_font;

  ALLEGRO_BITMAP      *lives_sprite;
  ALLEGRO_BITMAP      *asteroid_large;
  ALLEGRO_BITMAP      *asteroid_large_90;
  ALLEGRO_BITMAP      *asteroid_large_180;
  ALLEGRO_BITMAP      *asteroid_large_270;
  ALLEGRO_BITMAP      *asteroid_medium;
  ALLEGRO_BITMAP      *asteroid_medium_90;
  ALLEGRO_BITMAP      *asteroid_medium_180;
  ALLEGRO_BITMAP      *asteroid_medium_270;
  ALLEGRO_BITMAP      *asteroid_small;
  ALLEGRO_BITMAP      *asteroid_small_90;
  ALLEGRO_BITMAP      *asteroid_small_180;
  ALLEGRO_BITMAP      *asteroid_small_270;
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
  struct timeval t;
  gettimeofday(&t, NULL);
  srand(t.tv_usec * t.tv_sec);
  return low + (float) rand() / ((float) RAND_MAX / (high - low));
}

static bool
preload_asteroid_sprites(void)
{
  /* FIXME: fugtf */
  if((asteroids.asteroid_large = al_load_bitmap("data/sprites/asteroid-big.png")) == NULL)
    fprintf(stderr, "failed to load asteroid-big.png\n");
  if((asteroids.asteroid_large_90 = al_load_bitmap("data/sprites/asteroid-big-90.png")) == NULL)
    fprintf(stderr, "failed to load asteroid-big-90.png\n");
  if((asteroids.asteroid_large_180 = al_load_bitmap("data/sprites/asteroid-big-180.png")) == NULL)
    fprintf(stderr, "failed to load asteroid-big-180.png\n");
  if((asteroids.asteroid_large_270 = al_load_bitmap("data/sprites/asteroid-big-270.png")) == NULL)
    fprintf(stderr, "failed to load asteroid-big-270.png\n");
  if((asteroids.asteroid_medium = al_load_bitmap("data/sprites/asteroid-medium.png")) == NULL)
    fprintf(stderr, "failed to load asteroid-medium.png\n");
  if((asteroids.asteroid_medium_90 = al_load_bitmap("data/sprites/asteroid-medium-90.png")) == NULL)
    fprintf(stderr, "failed to load asteroid-medium-90.png\n");
  if((asteroids.asteroid_medium_180 = al_load_bitmap("data/sprites/asteroid-medium-180.png")) == NULL)
    fprintf(stderr, "failed to load asteroid-medium-180.png\n");
  if((asteroids.asteroid_medium_270 = al_load_bitmap("data/sprites/asteroid-medium-270.png")) == NULL)
    fprintf(stderr, "failed to load asteroid-medium-270.png\n");
  if((asteroids.asteroid_small = al_load_bitmap("data/sprites/asteroid-small.png")) == NULL)
    fprintf(stderr, "failed to load asteroid-small.png\n");
  if((asteroids.asteroid_small_90 = al_load_bitmap("data/sprites/asteroid-small-90.png")) == NULL)
    fprintf(stderr, "failed to load asteroid-small-90.png\n");
  if((asteroids.asteroid_small_180 = al_load_bitmap("data/sprites/asteroid-small-180.png")) == NULL)
    fprintf(stderr, "failed to load asteroid-small-180.png\n");
  if((asteroids.asteroid_small_270 = al_load_bitmap("data/sprites/asteroid-small-270.png")) == NULL)
    fprintf(stderr, "failed to load asteroid-small-270.png\n");

  return true;
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

  /* fonts */
  asteroids.small_font = al_load_ttf_font("data/vectorb.ttf", 12, 0);
  asteroids.large_font = al_load_ttf_font("data/vectorb.ttf", 24, 0);

  /* lives sprite */
  asteroids.lives_sprite = al_load_bitmap("data/sprites/ship.png");
  if(!asteroids.lives_sprite) {
    fprintf(stderr, "failed to load lives sprite.\n");
    return false;
  }

  /* preload asteroid sprites */
  if(!preload_asteroid_sprites()) {
    fprintf(stderr, "failed to preload asteroid spritse\n");
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

  al_hide_mouse_cursor(asteroids.display);

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

static struct ship *
create_ship(void)
{
  struct ship *ship = malloc(sizeof(struct ship));
  ship->position = malloc(sizeof(struct vector));
  ship->velocity = malloc(sizeof(struct vector));

  ship->sprite = al_load_bitmap("data/sprites/ship.png");
  if(!ship->sprite) {
    fprintf(stderr, "failed to create ship sprite.\n");
    return NULL;
  }

  ship->thrust_sprite = al_load_bitmap("data/sprites/ship-thrust.png");
  if(!ship->sprite) {
    fprintf(stderr, "failed to create ship (thrust) sprite.\n");
    return NULL;
  }

  ship->width       = al_get_bitmap_width(ship->sprite);
  ship->height      = al_get_bitmap_height(ship->sprite);
  ship->angle       = 0.0;
  ship->velocity->x = 0.0;
  ship->velocity->y = 0.0;
  ship->position->x = SCREEN_W / 2;
  ship->position->y = SCREEN_H / 2;
  ship->missiles = malloc(sizeof(struct misssile *) * MAX_MISSILES);
  ship->thrust_visible = false;

  int i;
  for(i = 0; i < MAX_MISSILES; i++)
    ship->missiles[i] = create_missile(ship);

  return ship;
}

static ALLEGRO_BITMAP *
load_asteroid_sprite(uint8_t size, float angle)
{
  ALLEGRO_BITMAP *sprite = NULL;

  /* what the fuck?
   *
   * rotated bitmaps are horribly pixelated
   * this is a hack to get nice, clean sprites
   *
   * also, this is _terribly_ messy. FIXME */
  switch(size) {
    case ASTEROID_LARGE:
      if(angle < 90.0)
        sprite = asteroids.asteroid_large;
      else if(angle < 180.0)
        sprite = asteroids.asteroid_large_90;
      else if(angle < 270.0)
        sprite = asteroids.asteroid_large_180;
      else if(angle < 360.0)
        sprite = asteroids.asteroid_large_270;
      break;
    case ASTEROID_MEDIUM:
      if(angle < 90.0)
        sprite = asteroids.asteroid_medium;
      else if(angle < 180.0)
        sprite = asteroids.asteroid_medium_90;
      else if(angle < 270.0)
        sprite = asteroids.asteroid_medium_180;
      else if(angle < 360.0)
        sprite = asteroids.asteroid_medium_270;
      break;
    case ASTEROID_SMALL:
      if(angle < 90.0)
        sprite = asteroids.asteroid_small;
      else if(angle < 180.0)
        sprite = asteroids.asteroid_small_90;
      else if(angle < 270.0)
        sprite = asteroids.asteroid_small_180;
      else if(angle < 360.0)
        sprite = asteroids.asteroid_small_270;
      break;
  }

  if(!sprite) {
    fprintf(stderr, "failed to load asteroid sprite.\n");
    return NULL;
  }

  return sprite;
}

static struct asteroid *
create_asteroid(uint8_t size)
{
  struct asteroid *asteroid = malloc(sizeof(struct asteroid));

  asteroid->position = malloc(sizeof(struct vector));
  asteroid->velocity = malloc(sizeof(struct vector));

  asteroid->size  = size;
  asteroid->angle = rand_f(0.0, 360.0);
  asteroid->position->x = rand_f(1.0, SCREEN_W);
  asteroid->position->y = rand_f(1.0, SCREEN_H);
  asteroid->velocity->x = (float)   sin(deg2rad(asteroid->angle))  * rand_f(0.1, 1.2);
  asteroid->velocity->y = (float) -(cos(deg2rad(asteroid->angle))) * rand_f(0.1, 1.2);

  switch(size) {
    case ASTEROID_LARGE:
      asteroid->points = ASTEROID_LARGE_POINTS;
      break;
    case ASTEROID_MEDIUM:
      asteroid->points = ASTEROID_MEDIUM_POINTS;
      break;
    case ASTEROID_SMALL:
      asteroid->points = ASTEROID_SMALL_POINTS;
      break;
  }

  if((asteroid->sprite = load_asteroid_sprite(size, asteroid->angle)) == NULL) {
    free(asteroid->position);
    free(asteroid->velocity);
    free(asteroid);

    return NULL;
  }

  asteroid->width  = al_get_bitmap_width(asteroid->sprite);
  asteroid->height = al_get_bitmap_height(asteroid->sprite);

  return asteroid;
}

static void
free_asteroid(struct asteroid *asteroid)
{
  if(asteroid->position != NULL)
    free(asteroid->position);
  if(asteroid->velocity != NULL)
    free(asteroid->velocity);
  if(asteroid != NULL)
    free(asteroid);

  asteroid = NULL;
}

static void
explode_asteroid(struct asteroid *asteroid, struct missile *missile)
{
  int i;
  struct vector position;
  struct level *level = asteroids.level;

  missile->active = false;
  asteroids.score += asteroid->points;

  position.x = asteroid->position->x;
  position.y = asteroid->position->y;

  /* find the asteroid to destory in the level */
  for(i = 0; i < level->n_asteroids; i++)
    if(level->asteroids[i] == asteroid)
      break;

  if(asteroid->size != ASTEROID_SMALL) {
    level->n_asteroids++;
    level->asteroids = (struct asteroid **) realloc(level->asteroids, sizeof(struct asteroid *) * level->n_asteroids);
    if(level->asteroids == NULL)
      fprintf(stderr, "unable to reallocate memory\n");

    /* replace the destroyed asteroid */
    free_asteroid(asteroid);
    if(asteroid->size == ASTEROID_LARGE) {
      asteroids.level->asteroids[i] = create_asteroid(ASTEROID_MEDIUM);
      asteroids.level->asteroids[level->n_asteroids - 1] = create_asteroid(ASTEROID_MEDIUM);
    } else {
      asteroids.level->asteroids[i] = create_asteroid(ASTEROID_SMALL);
      asteroids.level->asteroids[level->n_asteroids - 1] = create_asteroid(ASTEROID_SMALL);
    }

    asteroids.level->asteroids[i]->position->x = position.x;
    asteroids.level->asteroids[i]->position->y = position.y;
    asteroids.level->asteroids[level->n_asteroids - 1]->position->x = position.x;
    asteroids.level->asteroids[level->n_asteroids - 1]->position->y = position.y;
    return;
  } else {
    struct asteroid **temp = malloc(sizeof(struct asteroid *) * level->n_asteroids - 1);

    for(int i = 0, j = 0; i < level->n_asteroids; i++) {
      if(level->asteroids[i] != asteroid) {
        temp[j] = level->asteroids[i];
        j++;
      }
    }

    free_asteroid(asteroid);
    free(level->asteroids);
    level->asteroids = temp;
    level->n_asteroids--;

    return;
  }
}

static struct level *
create_level(int n_asteroids)
{
  struct level *level = malloc(sizeof(struct level));

  level->n_asteroids = n_asteroids;
  level->asteroids = malloc(sizeof(struct asteroid *) * n_asteroids);
  for(int i = 0; i < n_asteroids; i++)
    level->asteroids[i] = create_asteroid(ASTEROID_LARGE);

  return level;
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
draw_ship(struct ship *ship, bool thrusting)
{
  /* this creates a flashing thrust visualization
   * not _exactly_ like the original (too fast). (FIXME) */
  if(thrusting && !ship->thrust_visible) {
    al_draw_rotated_bitmap(
        ship->thrust_sprite,
        ship->width  / 2,
        ship->height / 2,
        ship->position->x,
        ship->position->y,
        deg2rad(ship->angle), 0);
    ship->thrust_visible = true;
  } else {
    al_draw_rotated_bitmap(
        ship->sprite,
        ship->width  / 2,
        ship->height / 2,
        ship->position->x,
        ship->position->y,
        deg2rad(ship->angle), 0);
    ship->thrust_visible = false;
  }
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
draw_score(void)
{
  char score[20];
  sprintf(score, "%02lu", asteroids.score);

  al_draw_text(asteroids.large_font,
      al_map_rgb(255,255,255),
      SCORE_X,
      SCORE_Y,
      ALLEGRO_ALIGN_RIGHT,
      score);
}

static void
draw_high_score(void)
{
  char score[20];
  sprintf(score, "%02lu", asteroids.high_score);

  al_draw_text(asteroids.small_font,
      al_map_rgb(255,255,255),
      SCREEN_W / 2,
      HIGH_SCORE_Y,
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

static bool
missile_collision(struct missile *missile, struct asteroid *asteroid)
{
  if(asteroid == NULL)
    return false;

  float missile_x = missile->position->x - (missile->width   / 2);
  float missile_y = missile->position->y - (missile->height  / 2);
  float rock_x = asteroid->position->x   - (asteroid->width  / 2);
  float rock_y = asteroid->position->y   - (asteroid->height / 2);

  return collision(missile_x, missile_y, missile->width, missile->height,
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
  bool debounce = false; /* force user to press for each fire */

  atexit(shutdown);

  if(!init())
    exit(EXIT_FAILURE);

  if((asteroids.ship = create_ship()) == NULL)
    exit(EXIT_FAILURE);

  asteroids.level = create_level(4);

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

      /* ship->asteroid collisions. */
      for(int i = 0; i < asteroids.level->n_asteroids; i++) {
        if(asteroid_collision(asteroids.ship, asteroids.level->asteroids[i])) {
          printf("ZOMG COLLISION!!!\n");
        }
      }

      /* missile->asteroid collisions. FIXME: who made this mess? */
      for(int i = 0; i < MAX_MISSILES; i++) {
        if(asteroids.ship->missiles[i]->active) {
          for(int j = 0; j < asteroids.level->n_asteroids; j++) {
            if(missile_collision(asteroids.ship->missiles[i], asteroids.level->asteroids[j])) {
              explode_asteroid(asteroids.level->asteroids[j], asteroids.ship->missiles[i]);
              i = 0;
              j = 0;
              continue;
            }
          }
        }
      }

      /* update positions */
      asteroids.ship->position->x += asteroids.ship->velocity->x;
      asteroids.ship->position->y += asteroids.ship->velocity->y;

      for(int i = 0; i < asteroids.level->n_asteroids; i++) {
        struct asteroid *asteroid = asteroids.level->asteroids[i];
        asteroid->position->x += asteroid->velocity->x;
        asteroid->position->y += asteroid->velocity->y;
      }

      /* screen wrap */
      wrap_position(asteroids.ship->position);
      for(int i = 0; i < asteroids.level->n_asteroids; i++)
        wrap_position(asteroids.level->asteroids[i]->position);

      al_clear_to_color(al_map_rgb(0, 0, 0));

      /* if the ship is moving, show fire. */
      draw_ship(asteroids.ship, key[KEY_UP]);

      for(int i = 0; i < asteroids.level->n_asteroids; i++)
        draw_asteroid(asteroids.level->asteroids[i]);
      draw_score();
      draw_high_score();
      draw_lives();

      for(int i = 0; i < MAX_MISSILES; i++) {
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

      /* slow down over time */
      drag(asteroids.ship);
    }
  }

  // cleanup
  if(asteroids.timer != NULL)
    al_destroy_timer(asteroids.timer);
  if(asteroids.event_queue != NULL)
    al_destroy_event_queue(asteroids.event_queue);
  if(asteroids.display != NULL)
    al_destroy_display(asteroids.display);

  for(int i = 0; i < MAX_MISSILES; i++)
    free_missile(asteroids.ship->missiles[i]);
  for(int i = 0; i < asteroids.level->n_asteroids; i++)
    free_asteroid(asteroids.level->asteroids[i]);
  free_ship(&asteroids.ship);

  al_destroy_bitmap(asteroids.asteroid_large);
  al_destroy_bitmap(asteroids.asteroid_large_90);
  al_destroy_bitmap(asteroids.asteroid_large_180);
  al_destroy_bitmap(asteroids.asteroid_large_270);
  al_destroy_bitmap(asteroids.asteroid_medium);
  al_destroy_bitmap(asteroids.asteroid_medium_90);
  al_destroy_bitmap(asteroids.asteroid_medium_180);
  al_destroy_bitmap(asteroids.asteroid_medium_270);
  al_destroy_bitmap(asteroids.asteroid_small);
  al_destroy_bitmap(asteroids.asteroid_small_90);
  al_destroy_bitmap(asteroids.asteroid_small_180);
  al_destroy_bitmap(asteroids.asteroid_small_270);


  exit(EXIT_SUCCESS);
}
