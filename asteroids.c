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
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#include "util.h"
#include "ship.h"
#include "level.h"
#include "missile.h"
#include "asteroid.h"
#include "animation.h"
#include "asteroids.h"

enum CONTROLS {
  KEY_UP,      /* thrust */
  KEY_LEFT,    /* rotate left */
  KEY_RIGHT,   /* rotate right */
  KEY_SPACE,   /* fire ze missiles */
  KEY_LCONTROL /* HYPERSPACE! */
};

struct asteroids {
  unsigned long int score;
  unsigned long int high_score;

  int lives;
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

void
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
  if(!asteroid_init())
    return false;

  for(int i = 0; i < 15; i++) {
    char name[255];
    sprintf(name, "data/sprites/asteroid/explosion/%d.png", i + 1);
    if((asteroids.explosion_sprites[i] = al_load_bitmap(name)) == NULL)
      fprintf(stderr, "failed to load explosion sprite %d\n", i);
  }

  asteroids.timer = al_create_timer(1.0 / FPS);
  if(!asteroids.timer) {
    fprintf(stderr, "failed to create timer.\n");
    return false;
  }

  asteroids.event_queue = al_create_event_queue();
  if(!asteroids.event_queue) {
    fprintf(stderr, "failed to create event queue.\n");
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

  al_register_event_source(asteroids.event_queue, al_get_display_event_source(asteroids.display));
  al_register_event_source(asteroids.event_queue, al_get_timer_event_source(asteroids.timer));
  al_register_event_source(asteroids.event_queue, al_get_keyboard_event_source());

  return true;
}

SHIP *
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

static void
new_explosion(VECTOR *position)
{
  ANIMATION *explosion = animation_new(asteroids.explosion_sprites, 15);

  explosion->slowdown = 2;
  explosion->position->x = position->x - (explosion->width  / 2);
  explosion->position->y = position->y - (explosion->height / 2);

  asteroids.n_explosions++;
  asteroids.explosions = (ANIMATION **) realloc(asteroids.explosions, sizeof(ANIMATION *) * asteroids.n_explosions);
  asteroids.explosions[asteroids.n_explosions - 1] = explosion;
}

void
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

  animation_free(explosion);
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

    asteroid_free(asteroid);
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

  asteroid_free(asteroid);
}

static void
missile_explode_asteroid(MISSILE *missile, ASTEROID *asteroid)
{
  missile->active = false;
  asteroids.score += asteroid->points;

  new_explosion(missile->position);
  explode_asteroid(asteroid);
}

int
main(void)
{
  asteroids.score       = 0;
  asteroids.lives       = START_LIVES;
  asteroids.display     = NULL;
  asteroids.timer       = NULL;
  asteroids.event_queue = NULL;
  SHIP *ship;

  bool redraw = true;
  bool quit   = false;
  bool key[5] = { false };

  seed_rand();
  atexit(shutdown);

  if(!init())
    exit(EXIT_FAILURE);

  if((ship = create_ship()) == NULL)
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
        ship_accelerate(ship);

      /* rotate */
      if(key[KEY_LEFT])
        ship_rotate(ship, -3);
      if(key[KEY_RIGHT])
        ship_rotate(ship, 3);

      /* hyperspace */
      if(key[KEY_LCONTROL])
        ship_hyperspace(ship);

      /* shoot */
      if(key[KEY_SPACE])
        ship_fire(ship, asteroids.timer);

      /* ship->asteroid collisions. */
      for(int i = 0; i < asteroids.level->n_asteroids; i++) {
        if(asteroid_collision(ship, asteroids.level->asteroids[i])) {
          asteroids.score += asteroids.level->asteroids[i]->points;
          explode_asteroid(asteroids.level->asteroids[i]);
          if(ship_explode(ship, asteroids.ship_explosion_sprites, 60))
            asteroids.lives--;
        }
      }

      /* missile->asteroid collisions. FIXME: who made this mess? */
      for(int i = 0; i < MAX_MISSILES; i++) {
        if(ship->missiles[i]->active) {
          for(int j = 0; j < asteroids.level->n_asteroids; j++) {
            if(missile_collision(ship->missiles[i], asteroids.level->asteroids[j])) {
              missile_explode_asteroid(ship->missiles[i], asteroids.level->asteroids[j]);
              i = 0;
              j = 0;
              continue;
            }
          }
        }
      }

      /* update positions */
      ship_update(ship);
      for(int i = 0; i < asteroids.level->n_asteroids; i++)
        asteroid_update(asteroids.level->asteroids[i]);
      for(int i = 0; i < MAX_MISSILES; i++)
        if(ship->missiles[i]->active)
          update_missile(ship->missiles[i], asteroids.timer);
      for(int i = 0; i < asteroids.n_explosions; i++)
        if(asteroids.explosions[i]->current_frame < asteroids.explosions[i]->n_frames)
          animation_update(asteroids.explosions[i]);
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
          ship->fire_debounce = false;
          break;
        case ALLEGRO_KEY_LCTRL:
          key[KEY_LCONTROL] = false;
          ship->hyper_debounce = false;
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
      ship_draw(ship, key[KEY_UP]);
      for(int i = 0; i < asteroids.level->n_asteroids; i++)
        asteroid_draw(asteroids.level->asteroids[i]);
      for(int i = 0; i < MAX_MISSILES; i++)
        if(ship->missiles[i]->active)
          missile_draw(ship->missiles[i]);
      for(int i = 0; i < asteroids.n_explosions; i++)
        animation_draw(asteroids.explosions[i]);

      al_flip_display();
    }
  };

  /* FIXME: cleanup */
  if(asteroids.timer != NULL)
    al_destroy_timer(asteroids.timer);
  if(asteroids.event_queue != NULL)
    al_destroy_event_queue(asteroids.event_queue);
  if(asteroids.display != NULL)
    al_destroy_display(asteroids.display);

  for(int i = 0; i < MAX_MISSILES; i++)
    missile_free(ship->missiles[i]);
  for(int i = 0; i < asteroids.level->n_asteroids; i++)
    asteroid_free(asteroids.level->asteroids[i]);
  ship_free(ship);

  al_destroy_bitmap(asteroids.lives_sprite);
  al_destroy_bitmap(asteroids.ship_sprite);
  al_destroy_bitmap(asteroids.ship_thrust_sprite);
  asteroid_shutdown();

  exit(EXIT_SUCCESS);
}
