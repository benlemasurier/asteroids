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
  KEY_UP,    /* thrust */
  KEY_LEFT,  /* rotate left */
  KEY_RIGHT, /* rotate right */
  KEY_SPACE  /* fire ze missiles */
};

typedef struct vector_t {
  float x;
  float y;
} VECTOR;

typedef struct missile_t {
  int width;
  int height;
  bool active;
  int64_t time;

  float angle;
  VECTOR *position;
  VECTOR *velocity;

  ALLEGRO_BITMAP *sprite;
} MISSILE;

typedef struct ship_t {
  int width;
  int height;
  bool thrust_visible;

  MISSILE **missiles;

  float angle;
  VECTOR *position;
  VECTOR *velocity;

  ALLEGRO_BITMAP *sprite;
  ALLEGRO_BITMAP *thrust_sprite;
} SHIP;

typedef struct asteroid_t {
  int width;
  int height;
  float angle;
  uint8_t size;
  uint8_t points;

  VECTOR *position;
  VECTOR *velocity;

  ALLEGRO_BITMAP *sprite;
} ASTEROID;

typedef struct explosion_t {
  uint8_t width;
  uint8_t height;
  uint8_t frame_played;
  uint8_t current_frame;
  VECTOR *position;
} EXPLOSION;

typedef struct level_t {
  int n_asteroids;
  ASTEROID **asteroids;
} LEVEL;

struct asteroids {
  unsigned long int   score;
  unsigned long int   high_score;

  int lives;
  SHIP  *ship;
  LEVEL *level;

  uint8_t   n_explosions;
  EXPLOSION **explosions;

  ALLEGRO_FONT    *small_font;
  ALLEGRO_FONT    *large_font;
  ALLEGRO_TIMER   *timer;
  ALLEGRO_DISPLAY *display;
  ALLEGRO_EVENT_QUEUE *event_queue;

  ALLEGRO_BITMAP *explosion_sprites[15];

  ALLEGRO_BITMAP *lives_sprite;
  ALLEGRO_BITMAP *asteroid_large;
  ALLEGRO_BITMAP *asteroid_large_90;
  ALLEGRO_BITMAP *asteroid_large_180;
  ALLEGRO_BITMAP *asteroid_large_270;
  ALLEGRO_BITMAP *asteroid_medium;
  ALLEGRO_BITMAP *asteroid_medium_90;
  ALLEGRO_BITMAP *asteroid_medium_180;
  ALLEGRO_BITMAP *asteroid_medium_270;
  ALLEGRO_BITMAP *asteroid_small;
  ALLEGRO_BITMAP *asteroid_small_90;
  ALLEGRO_BITMAP *asteroid_small_180;
  ALLEGRO_BITMAP *asteroid_small_270;
} asteroids;

static void
shutdown(void)
{
  /* FIXME: why can't I cleanly access asteroids.timer,display,etc here? */
  printf("shutdown.\n");
}

static void
rotate_ship(SHIP *ship, float deg)
{
  ship->angle += deg;

  if(ship->angle > 360.0)
    ship->angle -= 360.0;
  if(ship->angle < 0)
    ship->angle += 360.0;
}

static void
wrap_position(VECTOR *position)
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

static float
deg2rad(float deg)
{
  return deg * (M_PI / 180);
}

static float
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
  if((asteroids.asteroid_large = al_load_bitmap("data/sprites/asteroid/large/default.png")) == NULL)
    fprintf(stderr, "failed to load asteroid-large.png\n");
  if((asteroids.asteroid_large_90 = al_load_bitmap("data/sprites/asteroid/large/90.png")) == NULL)
    fprintf(stderr, "failed to load asteroid-large-90.png\n");
  if((asteroids.asteroid_large_180 = al_load_bitmap("data/sprites/asteroid/large/180.png")) == NULL)
    fprintf(stderr, "failed to load asteroid-large-180.png\n");
  if((asteroids.asteroid_large_270 = al_load_bitmap("data/sprites/asteroid/large/270.png")) == NULL)
    fprintf(stderr, "failed to load asteroid-large-270.png\n");
  if((asteroids.asteroid_medium = al_load_bitmap("data/sprites/asteroid/medium/default.png")) == NULL)
    fprintf(stderr, "failed to load asteroid-medium.png\n");
  if((asteroids.asteroid_medium_90 = al_load_bitmap("data/sprites/asteroid/medium/90.png")) == NULL)
    fprintf(stderr, "failed to load asteroid-medium-90.png\n");
  if((asteroids.asteroid_medium_180 = al_load_bitmap("data/sprites/asteroid/medium/180.png")) == NULL)
    fprintf(stderr, "failed to load asteroid-medium-180.png\n");
  if((asteroids.asteroid_medium_270 = al_load_bitmap("data/sprites/asteroid/medium/270.png")) == NULL)
    fprintf(stderr, "failed to load asteroid-medium-270.png\n");
  if((asteroids.asteroid_small = al_load_bitmap("data/sprites/asteroid/small/default.png")) == NULL)
    fprintf(stderr, "failed to load asteroid-small.png\n");
  if((asteroids.asteroid_small_90 = al_load_bitmap("data/sprites/asteroid/small/90.png")) == NULL)
    fprintf(stderr, "failed to load asteroid-small-90.png\n");
  if((asteroids.asteroid_small_180 = al_load_bitmap("data/sprites/asteroid/small/180.png")) == NULL)
    fprintf(stderr, "failed to load asteroid-small-180.png\n");
  if((asteroids.asteroid_small_270 = al_load_bitmap("data/sprites/asteroid/small/270.png")) == NULL)
    fprintf(stderr, "failed to load asteroid-small-270.png\n");

  for(int i = 0; i < 15; i++) {
    char name[255];
    sprintf(name, "data/sprites/asteroid/explosion/%d.png", i + 1);
    if((asteroids.explosion_sprites[i] = al_load_bitmap(name)) == NULL)
      fprintf(stderr, "failed to load explosion sprite %d\n", i);
  }

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

static MISSILE *
create_missile(void)
{
  MISSILE *missile  = malloc(sizeof(MISSILE));
  missile->position = malloc(sizeof(VECTOR));
  missile->velocity = malloc(sizeof(VECTOR));

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

static SHIP *
create_ship(void)
{
  SHIP *ship = malloc(sizeof(SHIP));
  ship->position = malloc(sizeof(VECTOR));
  ship->velocity = malloc(sizeof(VECTOR));

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

  for(int i = 0; i < MAX_MISSILES; i++)
    ship->missiles[i] = create_missile();

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

static ASTEROID *
create_asteroid(uint8_t size)
{
  ASTEROID *asteroid = malloc(sizeof(ASTEROID));

  asteroid->position = malloc(sizeof(VECTOR));
  asteroid->velocity = malloc(sizeof(VECTOR));

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
free_asteroid(ASTEROID *asteroid)
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
free_explosion(EXPLOSION *explosion)
{
  free(explosion->position);
  free(explosion);
  explosion = NULL;
}

static LEVEL *
create_level(int n_asteroids)
{
  LEVEL *level = malloc(sizeof(LEVEL));

  level->n_asteroids = n_asteroids;
  level->asteroids = malloc(sizeof(ASTEROID *) * n_asteroids);
  for(int i = 0; i < n_asteroids; i++)
    level->asteroids[i] = create_asteroid(ASTEROID_LARGE);

  return level;
}

static void
new_explosion(VECTOR *position)
{
  EXPLOSION *explosion = malloc(sizeof(EXPLOSION));
  explosion->width  = al_get_bitmap_width(asteroids.explosion_sprites[0]);
  explosion->height = al_get_bitmap_height(asteroids.explosion_sprites[0]);
  explosion->frame_played  = 0;
  explosion->current_frame = 0;
  explosion->position = malloc(sizeof(VECTOR));

  explosion->position->x = position->x - (explosion->width  / 2);
  explosion->position->y = position->y - (explosion->height / 2);

  asteroids.n_explosions++;
  asteroids.explosions = (EXPLOSION **) realloc(asteroids.explosions, sizeof(EXPLOSION *) * asteroids.n_explosions);
  asteroids.explosions[asteroids.n_explosions - 1] = explosion;
}

static void
remove_explosion(EXPLOSION *explosion)
{
  EXPLOSION **temp = malloc(sizeof(EXPLOSION *) * asteroids.n_explosions - 1);
  for(int i = 0, j = 0; i < asteroids.n_explosions; i++) {
    if(asteroids.explosions[i] != explosion) {
      temp[j] = asteroids.explosions[i];
      j++;
    }
  }


  free(asteroids.explosions);
  asteroids.explosions = temp;
  asteroids.n_explosions--;

  free_explosion(explosion);
}

static void
launch_missile(SHIP *ship, MISSILE *missile)
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
free_ship(SHIP *ship)
{
  if(ship->position != NULL)
    free(ship->position);
  if(ship->velocity != NULL)
    free(ship->velocity);

  if(ship->sprite != NULL)
    al_destroy_bitmap(ship->sprite);
  if(ship != NULL)
    free(ship);

  ship = NULL;
}

static void
free_missile(MISSILE *missile)
{
  if(missile == NULL)
    return;

  free(missile->position);
  free(missile->velocity);
  al_destroy_bitmap(missile->sprite);
  free(missile);

  missile = NULL;
}

static void
accelerate(SHIP *ship)
{
  ship->velocity->x += (float)   sin(deg2rad(ship->angle))  * ACCEL_SCALE;
  ship->velocity->y += (float) -(cos(deg2rad(ship->angle))) * ACCEL_SCALE;
}

static void
drag(SHIP *ship)
{
  ship->velocity->x *= DRAG;
  ship->velocity->y *= DRAG;
}

static void
draw_ship(SHIP *ship, bool thrusting)
{
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
      deg2rad(ship->angle), 0);

  ship->thrust_visible = (ship->thrust_visible) ? false : true;
}

static void
draw_asteroid(ASTEROID *asteroid)
{
  al_draw_bitmap(
      asteroid->sprite,
      asteroid->position->x - (asteroid->width  / 2),
      asteroid->position->y - (asteroid->height / 2),
      0);
}

static void
draw_missile(MISSILE *missile)
{
  al_draw_bitmap(
      missile->sprite,
      missile->position->x - (missile->width  / 2),
      missile->position->y - (missile->height / 2),
      0);
}

static void
draw_explosion(EXPLOSION *explosion)
{
  al_draw_bitmap(
      asteroids.explosion_sprites[explosion->current_frame],
      explosion->position->x,
      explosion->position->y,
      0);

  /* slow down explosion playback by rendering
   * each frame multiple times */
  if(explosion->frame_played < 4) {
    explosion->frame_played++;
  } else {
    explosion->current_frame++;
    explosion->frame_played = 0;
  }
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
asteroid_collision(SHIP *ship, ASTEROID *asteroid)
{
  float ship_x = ship->position->x - (ship->width  / 2);
  float ship_y = ship->position->y - (ship->height / 2);
  float rock_x = asteroid->position->x - (asteroid->width  / 2);
  float rock_y = asteroid->position->y - (asteroid->height / 2);

  return collision(ship_x, ship_y, ship->width, ship->height,
                   rock_x, rock_y, asteroid->width, asteroid->height);
}

static bool
missile_collision(MISSILE *missile, ASTEROID *asteroid)
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

static void
explode_asteroid(ASTEROID *asteroid, MISSILE *missile)
{
  int i, j;
  VECTOR position;
  LEVEL *level = asteroids.level;

  missile->active = false;
  asteroids.score += asteroid->points;

  new_explosion(missile->position);

  position.x = asteroid->position->x;
  position.y = asteroid->position->y;

  if(asteroid->size == ASTEROID_SMALL) {
    ASTEROID **temp = malloc(sizeof(ASTEROID *) * level->n_asteroids - 1);

    for(i = 0, j = 0; i < level->n_asteroids; i++) {
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

  /* find the asteroid to destory in the level */
  for(i = 0; i < level->n_asteroids; i++)
    if(level->asteroids[i] == asteroid)
      break;

  level->n_asteroids++;
  level->asteroids = (ASTEROID **) realloc(level->asteroids, sizeof(ASTEROID *) * level->n_asteroids);
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
}

static void
update_ship(SHIP *ship)
{
  ship->position->x += ship->velocity->x;
  ship->position->y += ship->velocity->y;
  wrap_position(asteroids.ship->position);

  /* slow down over time */
  drag(asteroids.ship);
}

static void
update_asteroid(ASTEROID *asteroid)
{
  asteroid->position->x += asteroid->velocity->x;
  asteroid->position->y += asteroid->velocity->y;
  wrap_position(asteroid->position);
}

static void
update_missile(MISSILE *missile)
{
  if((missile->time + (MISSILE_TTL * FPS)) < al_get_timer_count(asteroids.timer)) {
    missile->active = false;
    return;
  }

  missile->position->x += missile->velocity->x;
  missile->position->y += missile->velocity->y;
  wrap_position(missile->position);
}

int
main(void)
{
  asteroids.score       = 0;
  asteroids.lives       = START_LIVES;
  asteroids.display     = NULL;
  asteroids.timer       = NULL;
  asteroids.event_queue = NULL;

  bool redraw   = true;
  bool quit     = false;
  bool debounce = false; /* force button press for each fire */
  bool key[4]   = { false, false, false, false };

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
      /* move forward */
      if(key[KEY_UP])
        accelerate(asteroids.ship);

      /* rotate */
      if(key[KEY_LEFT])
        rotate_ship(asteroids.ship, -3);
      if(key[KEY_RIGHT])
        rotate_ship(asteroids.ship, 3);

      /* shoot */
      if(key[KEY_SPACE]) {
        for(int i = 0; i < MAX_MISSILES; i++) {
          if(!asteroids.ship->missiles[i]->active && !debounce) {
            launch_missile(asteroids.ship, asteroids.ship->missiles[i]);
            debounce = true;
            break;
          }
        }
      }

      /* ship->asteroid collisions. */
      for(int i = 0; i < asteroids.level->n_asteroids; i++)
        if(asteroid_collision(asteroids.ship, asteroids.level->asteroids[i]))
          printf("ZOMG collision\n");

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
      update_ship(asteroids.ship);
      for(int i = 0; i < asteroids.level->n_asteroids; i++)
        update_asteroid(asteroids.level->asteroids[i]);
      for(int i = 0; i < MAX_MISSILES; i++)
        if(asteroids.ship->missiles[i]->active)
          update_missile(asteroids.ship->missiles[i]);
      for(int i = 0; i < asteroids.n_explosions; i++)
        if(asteroids.explosions[i]->current_frame > 14)
          remove_explosion(asteroids.explosions[i]);

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
      al_clear_to_color(al_map_rgb(0, 0, 0));

      draw_score();
      draw_high_score();
      draw_lives();
      draw_ship(asteroids.ship, key[KEY_UP]);
      for(int i = 0; i < asteroids.level->n_asteroids; i++)
        draw_asteroid(asteroids.level->asteroids[i]);
      for(int i = 0; i < MAX_MISSILES; i++)
        if(asteroids.ship->missiles[i]->active)
          draw_missile(asteroids.ship->missiles[i]);
      for(int i = 0; i < asteroids.n_explosions; i++)
        draw_explosion(asteroids.explosions[i]);

      al_flip_display();
    }
  }

  /* FIXME: cleanup */
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
  free_ship(asteroids.ship);

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
