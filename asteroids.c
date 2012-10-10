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

#define FPS     60
#define DRAG    0.99

#define SCREEN_W      800
#define SCREEN_H      600
#define ACCEL_SCALE   0.07
#define MISSILE_SPEED 8
#define MISSILE_TTL   1
#define MAX_MISSILES  4
#define START_LIVES   3
#define LIVES_X       84
#define LIVES_Y       60
#define SCORE_X       130
#define SCORE_Y       27
#define HIGH_SCORE_Y  30

#define ASTEROID_LARGE  2
#define ASTEROID_MEDIUM 1
#define ASTEROID_SMALL  0

#define ASTEROID_LARGE_POINTS  20
#define ASTEROID_MEDIUM_POINTS 50
#define ASTEROID_SMALL_POINTS  100

#define DRAWING_FLAGS 0

enum CONTROLS {
  KEY_UP,      /* thrust */
  KEY_LEFT,    /* rotate left */
  KEY_RIGHT,   /* rotate right */
  KEY_SPACE,   /* fire ze missiles */
  KEY_LCONTROL /* HYPERSPACE! */
};

enum {
  LARGE       = 0,
  LARGE_90    = 1,
  LARGE_180   = 2,
  LARGE_270   = 3,
  MEDIUM      = 4,
  MEDIUM_90   = 5,
  MEDIUM_180  = 6,
  MEDIUM_270  = 7,
  SMALL       = 8,
  SMALL_90    = 9,
  SMALL_180   = 10,
  SMALL_270   = 11
};

typedef struct vector_t {
  float x;
  float y;
} VECTOR;

typedef struct animation_t {
  uint8_t width;
  uint8_t height;
  VECTOR *position;

  size_t  n_frames;
  size_t  current_frame;

  /* slowdown factor, play each frame `slowdown` times */
  uint8_t slowdown;

  /* how many times has the current frame been played? */
  uint8_t frame_played;

  ALLEGRO_BITMAP **sprites;
} ANIMATION;

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
  bool fire_debounce;

  MISSILE **missiles;

  float angle;
  VECTOR *position;
  VECTOR *velocity;

  ANIMATION *explosion;

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
  ANIMATION **explosions;

  ALLEGRO_FONT    *small_font;
  ALLEGRO_FONT    *large_font;
  ALLEGRO_TIMER   *timer;
  ALLEGRO_DISPLAY *display;
  ALLEGRO_EVENT_QUEUE *event_queue;

  ALLEGRO_BITMAP *ship_sprite;
  ALLEGRO_BITMAP *ship_thrust_sprite;
  ALLEGRO_BITMAP *lives_sprite;
  ALLEGRO_BITMAP *asteroid_sprites[12];
  ALLEGRO_BITMAP *explosion_sprites[15];
  ALLEGRO_BITMAP *ship_explosion_sprites[60];
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
  return low + (float) rand() / ((float) RAND_MAX / (high - low));
}

static bool
preload_asteroid_sprites(void)
{
  /* FIXME: fugtf */
  if((asteroids.asteroid_sprites[LARGE] = al_load_bitmap("data/sprites/asteroid/large/default.png")) == NULL)
    fprintf(stderr, "failed to load large/default.png\n");
  if((asteroids.asteroid_sprites[LARGE_90] = al_load_bitmap("data/sprites/asteroid/large/90.png")) == NULL)
    fprintf(stderr, "failed to load large/90.png\n");
  if((asteroids.asteroid_sprites[LARGE_180] = al_load_bitmap("data/sprites/asteroid/large/180.png")) == NULL)
    fprintf(stderr, "failed to load large/180.png\n");
  if((asteroids.asteroid_sprites[LARGE_270] = al_load_bitmap("data/sprites/asteroid/large/270.png")) == NULL)
    fprintf(stderr, "failed to load large/270.png\n");
  if((asteroids.asteroid_sprites[MEDIUM] = al_load_bitmap("data/sprites/asteroid/medium/default.png")) == NULL)
    fprintf(stderr, "failed to load medium/default.png\n");
  if((asteroids.asteroid_sprites[MEDIUM_90] = al_load_bitmap("data/sprites/asteroid/medium/90.png")) == NULL)
    fprintf(stderr, "failed to load medium/90.png\n");
  if((asteroids.asteroid_sprites[MEDIUM_180] = al_load_bitmap("data/sprites/asteroid/medium/180.png")) == NULL)
    fprintf(stderr, "failed to load medium/180.png\n");
  if((asteroids.asteroid_sprites[MEDIUM_270] = al_load_bitmap("data/sprites/asteroid/medium/270.png")) == NULL)
    fprintf(stderr, "failed to load medium/270.png\n");
  if((asteroids.asteroid_sprites[SMALL] = al_load_bitmap("data/sprites/asteroid/small/default.png")) == NULL)
    fprintf(stderr, "failed to load small/default.png\n");
  if((asteroids.asteroid_sprites[SMALL_90] = al_load_bitmap("data/sprites/asteroid/small/90.png")) == NULL)
    fprintf(stderr, "failed to load small/90.png\n");
  if((asteroids.asteroid_sprites[SMALL_180] = al_load_bitmap("data/sprites/asteroid/small/180.png")) == NULL)
    fprintf(stderr, "failed to load small/180.png\n");
  if((asteroids.asteroid_sprites[SMALL_270] = al_load_bitmap("data/sprites/asteroid/small/270.png")) == NULL)
    fprintf(stderr, "failed to load small/270.png\n");

  for(int i = 0; i < 15; i++) {
    char name[255];
    sprintf(name, "data/sprites/asteroid/explosion/%d.png", i + 1);
    if((asteroids.explosion_sprites[i] = al_load_bitmap(name)) == NULL)
      fprintf(stderr, "failed to load explosion sprite %d\n", i);
  }

  return true;
}

static bool
preload_ship_sprites(void)
{
  if((asteroids.ship_sprite = al_load_bitmap("data/sprites/ship.png")) == NULL) {
    fprintf(stderr, "failed to load ship sprite\n");
    return false;
  }

  if((asteroids.ship_thrust_sprite = al_load_bitmap("data/sprites/ship-thrust.png")) == NULL) {
    fprintf(stderr, "failed to load ship thrust sprite\n");
    return false;
  }

  /* ship explosion animation frames */
  for(int i = 0; i < 60; i++) {
    char name[255];
    sprintf(name, "data/sprites/ship/explosion/%d.png", i + 1);
    if((asteroids.ship_explosion_sprites[i] = al_load_bitmap(name)) == NULL) {
      fprintf(stderr, "failed to load ship explosion sprite %d\n", i);
      return false;
    }
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

  /* sprite preloading */
  if(!preload_ship_sprites())
    return false;
  if(!preload_asteroid_sprites())
    return false;

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

  /* TODO: show on mouse movement */
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

  ship->sprite = asteroids.ship_sprite;
  ship->thrust_sprite = asteroids.ship_thrust_sprite;

  ship->width       = al_get_bitmap_width(ship->sprite);
  ship->height      = al_get_bitmap_height(ship->sprite);
  ship->angle       = 0.0;
  ship->velocity->x = 0.0;
  ship->velocity->y = 0.0;
  ship->position->x = SCREEN_W / 2;
  ship->position->y = SCREEN_H / 2;
  ship->explosion   = NULL;
  ship->missiles = malloc(sizeof(struct misssile *) * MAX_MISSILES);
  ship->thrust_visible = false;
  ship->fire_debounce  = false;

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
        sprite = asteroids.asteroid_sprites[LARGE];
      else if(angle < 180.0)
        sprite = asteroids.asteroid_sprites[LARGE_90];
      else if(angle < 270.0)
        sprite = asteroids.asteroid_sprites[LARGE_180];
      else if(angle < 360.0)
        sprite = asteroids.asteroid_sprites[LARGE_270];
      break;
    case ASTEROID_MEDIUM:
      if(angle < 90.0)
        sprite = asteroids.asteroid_sprites[MEDIUM];
      else if(angle < 180.0)
        sprite = asteroids.asteroid_sprites[MEDIUM_90];
      else if(angle < 270.0)
        sprite = asteroids.asteroid_sprites[MEDIUM_180];
      else if(angle < 360.0)
        sprite = asteroids.asteroid_sprites[MEDIUM_270];
      break;
    case ASTEROID_SMALL:
      if(angle < 90.0)
        sprite = asteroids.asteroid_sprites[SMALL];
      else if(angle < 180.0)
        sprite = asteroids.asteroid_sprites[SMALL_90];
      else if(angle < 270.0)
        sprite = asteroids.asteroid_sprites[SMALL_180];
      else if(angle < 360.0)
        sprite = asteroids.asteroid_sprites[SMALL_270];
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
  free(asteroid->position);
  free(asteroid->velocity);
  free(asteroid);

  asteroid = NULL;
}

static void
free_animation(ANIMATION *animation)
{
  free(animation->position);
  free(animation);
  animation = NULL;
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

static ANIMATION *
new_animation(ALLEGRO_BITMAP **sprites, size_t n_frames)
{
  ANIMATION *animation = malloc(sizeof(ANIMATION));

  animation->width  = al_get_bitmap_width(sprites[0]);
  animation->height = al_get_bitmap_height(sprites[0]);
  animation->current_frame = 0;
  animation->frame_played  = 0;
  animation->slowdown      = 1;

  animation->position      = malloc(sizeof(VECTOR));
  animation->position->x   = 0;
  animation->position->y   = 0;

  animation->n_frames = n_frames;
  animation->sprites  = sprites;

  return animation;
}

static void
new_explosion(VECTOR *position)
{
  ANIMATION *explosion = new_animation(asteroids.explosion_sprites, 15);

  explosion->slowdown = 2;
  explosion->position->x = position->x - (explosion->width  / 2);
  explosion->position->y = position->y - (explosion->height / 2);

  asteroids.n_explosions++;
  asteroids.explosions = (ANIMATION **) realloc(asteroids.explosions, sizeof(ANIMATION *) * asteroids.n_explosions);
  asteroids.explosions[asteroids.n_explosions - 1] = explosion;
}

static void
ship_explode(SHIP *ship)
{
  if(ship->explosion != NULL)
    return;

  ANIMATION *explosion = new_animation(asteroids.ship_explosion_sprites, 60);

  // explosion->slowdown = 10;
  explosion->position->x = ship->position->x - (explosion->width  / 2);
  explosion->position->y = ship->position->y - (explosion->height / 2);

  ship->explosion = explosion;
  asteroids.lives--;
}

static void
remove_explosion(ANIMATION *explosion)
{
  ANIMATION **temp = malloc(sizeof(ANIMATION *) * asteroids.n_explosions - 1);
  for(int i = 0, j = 0; i < asteroids.n_explosions; i++) {
    if(asteroids.explosions[i] != explosion) {
      temp[j] = asteroids.explosions[i];
      j++;
    }
  }

  free(asteroids.explosions);
  asteroids.explosions = temp;
  asteroids.n_explosions--;

  free_animation(explosion);
}

static void
launch_missile(SHIP *ship)
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

  missile->time = al_get_timer_count(asteroids.timer);
}

static void
free_ship(SHIP *ship)
{
  if(ship->explosion != NULL)
    free_animation(ship->explosion);

  free(ship->position);
  free(ship->velocity);
  free(ship);

  ship = NULL;
}

static void
free_missile(MISSILE *missile)
{
  free(missile->position);
  free(missile->velocity);
  al_destroy_bitmap(missile->sprite);
  free(missile);

  missile = NULL;
}

static void
hyperspace(SHIP *ship)
{
  ship->velocity->x = 0.0;
  ship->velocity->y = 0.0;
  ship->position->x = rand_f(0, SCREEN_W);
  ship->position->y = rand_f(0, SCREEN_H);
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
draw_animation(ANIMATION *animation)
{
  if(animation->current_frame >= animation->n_frames)
    return;

  al_draw_bitmap(
      animation->sprites[animation->current_frame],
      animation->position->x,
      animation->position->y,
      DRAWING_FLAGS);
}

static void
draw_ship(SHIP *ship, bool thrusting)
{
  if(ship->explosion != NULL) {
    draw_animation(ship->explosion);
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

static void
draw_asteroid(ASTEROID *asteroid)
{
  al_draw_bitmap(
      asteroid->sprite,
      asteroid->position->x - (asteroid->width  / 2),
      asteroid->position->y - (asteroid->height / 2),
      DRAWING_FLAGS);
}

static void
draw_missile(MISSILE *missile)
{
  al_draw_bitmap(
      missile->sprite,
      missile->position->x - (missile->width  / 2),
      missile->position->y - (missile->height / 2),
      DRAWING_FLAGS);
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
        DRAWING_FLAGS);
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
explode_asteroid(ASTEROID *asteroid)
{
  int i, j;
  LEVEL *level = asteroids.level;

  if(asteroid->size == ASTEROID_SMALL) {
    ASTEROID **temp = malloc(sizeof(ASTEROID *) * level->n_asteroids - 1);

    for(i = 0, j = 0; i < level->n_asteroids; i++)
      if(level->asteroids[i] != asteroid)
        temp[j] = level->asteroids[i], j++;

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

  /* replace the asteroid to be destroyed and create another */
  asteroids.level->asteroids[i] = create_asteroid(asteroid->size - 1);
  asteroids.level->asteroids[level->n_asteroids - 1] = create_asteroid(asteroid->size - 1);

  asteroids.level->asteroids[i]->position->x = asteroid->position->x;
  asteroids.level->asteroids[i]->position->y = asteroid->position->y;
  asteroids.level->asteroids[level->n_asteroids - 1]->position->x = asteroid->position->x;
  asteroids.level->asteroids[level->n_asteroids - 1]->position->y = asteroid->position->y;

  free_asteroid(asteroid);
}

static void
missile_explode_asteroid(MISSILE *missile, ASTEROID *asteroid)
{
  missile->active = false;
  asteroids.score += asteroid->points;

  new_explosion(missile->position);
  explode_asteroid(asteroid);
}

static void
update_animation(ANIMATION *animation)
{
  /* slow down animation playback by rendering
   * each frame multiple times */
  if(animation->frame_played < animation->slowdown) {
    animation->frame_played++;

    return;
  }

  animation->current_frame++;
  animation->frame_played = 0;
}

static void
update_ship(SHIP *ship)
{
  if(ship->explosion != NULL) {
    update_animation(ship->explosion);

    /* if the animation is complete, create a new ship */
    if(ship->explosion->current_frame >= ship->explosion->n_frames) {
      free_ship(ship);
      /* FIXME: need preemptive collision detection, wait() */
      asteroids.ship = create_ship();
    }

    return;
  }

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

  bool redraw = true;
  bool quit   = false;
  bool key[5] = { false, false, false, false, false };

  /* force full button press */
  bool hyper_debounce = false;

  struct timeval t;
  gettimeofday(&t, NULL);
  srand(t.tv_usec * t.tv_sec);
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

      /* hyperspace */
      if(key[KEY_LCONTROL] && !hyper_debounce) {
        hyperspace(asteroids.ship);
        hyper_debounce = true;
      }

      /* shoot */
      if(key[KEY_SPACE])
        launch_missile(asteroids.ship);

      /* ship->asteroid collisions. */
      for(int i = 0; i < asteroids.level->n_asteroids; i++) {
        if(asteroid_collision(asteroids.ship, asteroids.level->asteroids[i])) {
          asteroids.score += asteroids.level->asteroids[i]->points;
          explode_asteroid(asteroids.level->asteroids[i]);
          ship_explode(asteroids.ship);
        }
      }

      /* missile->asteroid collisions. FIXME: who made this mess? */
      for(int i = 0; i < MAX_MISSILES; i++) {
        if(asteroids.ship->missiles[i]->active) {
          for(int j = 0; j < asteroids.level->n_asteroids; j++) {
            if(missile_collision(asteroids.ship->missiles[i], asteroids.level->asteroids[j])) {
              missile_explode_asteroid(asteroids.ship->missiles[i], asteroids.level->asteroids[j]);
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
        if(asteroids.explosions[i]->current_frame < asteroids.explosions[i]->n_frames)
          update_animation(asteroids.explosions[i]);
        else
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
        case ALLEGRO_KEY_LCTRL:
          key[KEY_LCONTROL] = true;
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
          asteroids.ship->fire_debounce = false;
          break;
        case ALLEGRO_KEY_LCTRL:
          key[KEY_LCONTROL] = false;
          hyper_debounce = false;
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
        draw_animation(asteroids.explosions[i]);

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

  al_destroy_bitmap(asteroids.lives_sprite);
  al_destroy_bitmap(asteroids.ship_sprite);
  al_destroy_bitmap(asteroids.ship_thrust_sprite);
  al_destroy_bitmap(asteroids.asteroid_sprites[LARGE]);
  al_destroy_bitmap(asteroids.asteroid_sprites[LARGE_90]);
  al_destroy_bitmap(asteroids.asteroid_sprites[LARGE_180]);
  al_destroy_bitmap(asteroids.asteroid_sprites[LARGE_270]);
  al_destroy_bitmap(asteroids.asteroid_sprites[MEDIUM]);
  al_destroy_bitmap(asteroids.asteroid_sprites[MEDIUM_90]);
  al_destroy_bitmap(asteroids.asteroid_sprites[MEDIUM_180]);
  al_destroy_bitmap(asteroids.asteroid_sprites[MEDIUM_270]);
  al_destroy_bitmap(asteroids.asteroid_sprites[SMALL]);
  al_destroy_bitmap(asteroids.asteroid_sprites[SMALL_90]);
  al_destroy_bitmap(asteroids.asteroid_sprites[SMALL_180]);
  al_destroy_bitmap(asteroids.asteroid_sprites[SMALL_270]);

  exit(EXIT_SUCCESS);
}
