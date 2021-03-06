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
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

#include "list.h"
#include "util.h"
#include "ship.h"
#include "level.h"
#include "saucer.h"
#include "missile.h"
#include "asteroid.h"
#include "animation.h"
#include "explosion.h"
#include "asteroids.h"
#include "configuration.h"

static struct asteroids {
  const char *high_score;

  int lives;
  LEVEL *level;

  LIST *explosions;

  ALLEGRO_FONT    *small_font;
  ALLEGRO_FONT    *large_font;
  ALLEGRO_TIMER   *timer;
  ALLEGRO_DISPLAY *display;
  ALLEGRO_EVENT_QUEUE *event_queue;

  ALLEGRO_BITMAP *lives_sprite;
} asteroids;

static void
score(int points)
{
  asteroids.level->score += points;
  if(asteroids.level->score > (unsigned long) atol(asteroids.high_score)) {
    char score_s[20];
    sprintf(score_s, "%02lu", asteroids.level->score);

    set_config_value("high_score", score_s);
  }
}

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

bool
offscreen(VECTOR *position, uint8_t width, uint8_t height)
{
  if((position->x - (width / 2)) > SCREEN_W ||
     (position->x + (width / 2)) < 0)
    return true;
  if((position->y - (height / 2)) > SCREEN_H ||
     (position->y + (height / 2)) < 0)
    return true;

  return false;
}

static void
draw_gameover(void)
{
  al_draw_text(asteroids.large_font,
      al_map_rgb(WHITE),
      SCREEN_W / 2,
      SCREEN_H / 3,
      ALLEGRO_ALIGN_CENTRE,
      "GAME OVER");
}

static void
draw_high_score(void)
{
  char score_s[20];
  sprintf(score_s, "%02lu", (unsigned long) atol(asteroids.high_score));

  al_draw_text(asteroids.small_font,
      al_map_rgb(WHITE),
      SCREEN_W / 2,
      HIGH_SCORE_Y,
      ALLEGRO_ALIGN_CENTRE,
      score_s);
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

static void
new_explosion(VECTOR *position)
{
  ANIMATION *explosion = explosion_create(position);
  asteroids.explosions = list_append(asteroids.explosions, explosion);
}

static void
remove_explosion(ANIMATION *explosion)
{
  asteroids.explosions = list_remove(asteroids.explosions, explosion);
  animation_free(explosion);
}

static void
explosions_update(void)
{
  LIST *head = list_first(asteroids.explosions);
  while(head) {
    ANIMATION *explosion = (ANIMATION *) head->data;

    if(explosion->current_frame < explosion->n_frames) {
      animation_update(explosion);
    } else {
      remove_explosion(explosion);
      head = list_first(asteroids.explosions);
      continue;
    }

    head = head->next;
  }
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

  /* sound */
  if(!al_install_audio()) {
    fprintf(stderr, "failed to initialize audio system.\n");
    return false;
  }
  if(!al_init_acodec_addon()) {
    fprintf(stderr, "failed to initialize audio codecs.\n");
    return false;
  }
  if(!al_reserve_samples(10)) {
    fprintf(stderr, "failed to reserve audio samples.\n");
    return false;
  }

  /* fonts */
  asteroids.small_font = al_load_ttf_font("data/vectorb.ttf", 12, 0);
  asteroids.large_font = al_load_ttf_font("data/vectorb.ttf", 24, 0);

  /* lives sprite */
  asteroids.lives_sprite = al_load_bitmap("data/sprites/ship/ship.png");
  if(!asteroids.lives_sprite) {
    fprintf(stderr, "failed to load lives sprite.\n");
    return false;
  }

  /* sprite preloading */
  if(!level_init())
    return false;
  if(!ship_init())
    return false;
  if(!missile_init())
    return false;
  if(!saucer_init())
    return false;
  if(!asteroid_init())
    return false;
  if(!explosion_init())
    return false;

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

  if(FULLSCREEN)
    al_set_new_display_flags(ALLEGRO_FULLSCREEN);
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

bool
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
ship_missile_saucer_collision(MISSILE *missile, SAUCER *saucer)
{
  float missile_x = missile->position->x - (missile->width   / 2);
  float missile_y = missile->position->y - (missile->height  / 2);
  float saucer_x = saucer->position->x - (saucer->width  / 2);
  float saucer_y = saucer->position->y - (saucer->height / 2);

  return collision(missile_x, missile_y, missile->width, missile->height,
      saucer_x, saucer_y, saucer->width, saucer->height);
}

static bool
saucer_missile_ship_collision(MISSILE *missile, SHIP *ship)
{
  float missile_x = missile->position->x - (missile->width   / 2);
  float missile_y = missile->position->y - (missile->height  / 2);
  float ship_x = ship->position->x - (ship->width  / 2);
  float ship_y = ship->position->y - (ship->height / 2);

  return collision(missile_x, missile_y, missile->width, missile->height,
      ship_x, ship_y, ship->width, ship->height);
}

static void
explode_asteroid(ASTEROID *asteroid)
{
  LEVEL *level = asteroids.level;

  level->asteroids = list_remove(level->asteroids, asteroid);

  /* if we're 'splitting' the asteroid, attempt to offset the new
   * asteroids from the path of seqential missiles */
  if(asteroid->size > ASTEROID_SMALL) {
    ASTEROID *tmp = asteroid_create(asteroid->size - 1);
    tmp->position->x = asteroid->position->x + rand_f(-6, 6);
    tmp->position->y = asteroid->position->y + rand_f(-6, 6);
    level->asteroids = list_append(level->asteroids, tmp);

    tmp = asteroid_create(asteroid->size - 1);
    tmp->position->x = asteroid->position->x + rand_f(-6, 6);;
    tmp->position->y = asteroid->position->y + rand_f(-6, 6);;
    level->asteroids = list_append(level->asteroids, tmp);
  }

  asteroid_free(asteroid);
}

static void
missile_explode_asteroid(MISSILE *missile, ASTEROID *asteroid)
{
  missile->active = false;
  score(asteroid->points);

  new_explosion(missile->position);
  explode_asteroid(asteroid);
}

int64_t
seconds_elapsed(int64_t time_count)
{
  /* FIXME: this is correct at 60FPS, but probably
     not how timers actually work */
  return(time_count / FPS);
}

static void
check_ship_asteroid_collisions(SHIP *ship, LIST *rocks)
{
  /* don't let the ship explode if it's already going down. */
  if(ship->explosion)
    return;

  LIST *head = list_first(rocks);
  while(head) {
    ASTEROID *a = (ASTEROID *) head->data;

    if(ship_asteroid_collision(ship, a)) {
      score(a->points);
      explode_asteroid(a);

      if(ship_explode(ship))
        asteroids.lives--;

      head = list_first(rocks);
    }

    head = head->next;
  }
}

static void
check_ship_missile_asteroid_collisions(LEVEL *level, SHIP *ship)
{
  LIST *head = list_first(ship->missiles);
  while(head) {
    MISSILE *m = (MISSILE *) head->data;

    if(!m->active) {
      head = head->next;
      continue;
    }

    LIST *rock = list_first(level->asteroids);
    while(rock) {
      ASTEROID *a = (ASTEROID *) rock->data;

      if(missile_asteroid_collision(m, a)) {
        missile_explode_asteroid(m, a);
        rock = list_first(level->asteroids);
        continue;
      }

      rock = rock->next;
    }

    head = head->next;
  }
}

static void
check_ship_missile_saucer_collisions(LEVEL *level, SHIP *ship)
{
  if(!level->saucer)
    return;

  LIST *head = list_first(ship->missiles);
  while(head) {
    MISSILE *m = (MISSILE *) head->data;

    if(ship_missile_saucer_collision(m, level->saucer)) {
      new_explosion(level->saucer->position);
      saucer_free(level->saucer);
      level->saucer = NULL;
      break;
    }

    head = head->next;
  }
}

static void
check_saucer_missile_ship_collisions(LEVEL *level, SHIP *ship)
{
  if(!level->saucer)
    return;

  if(!level->saucer->missile->active)
    return;

  MISSILE *m = level->saucer->missile;
  if(!saucer_missile_ship_collision(m, ship))
    return;

  asteroids.lives--; /* FIXME: global. */
  ship_explode(ship);
  saucer_exit(level->saucer);
}

static void
check_saucer_missile_asteroids_collisions(LEVEL *level)
{
  if(!level->saucer)
    return;

  MISSILE *m = level->saucer->missile;
  if(!m->active)
    return;

  LIST *rocks = list_first(level->asteroids);
  while(rocks) {
    ASTEROID *a = (ASTEROID *) rocks->data;

    if(missile_asteroid_collision(m, a)) {
      missile_explode_asteroid(m, a);
      break;
    }

    rocks = rocks->next;
  }
}

static void
check_saucer_asteroid_collisions(LEVEL *level)
{
  if(!level->saucer)
    return;

  LIST *head = list_first(level->asteroids);
  while(head && level->saucer) {
    ASTEROID *a = (ASTEROID *) head->data;

    if(saucer_asteroid_collision(level->saucer, a)) {
      new_explosion(a->position);
      explode_asteroid(a);
      saucer_free(level->saucer);
      level->saucer = NULL;
    }

    head = head->next;
  }
}

static void
draw_home(void)
{
  al_draw_text(asteroids.large_font,
      al_map_rgb(WHITE),
      SCREEN_W / 2,
      HIGH_SCORE_Y + 70,
      ALLEGRO_ALIGN_CENTRE,
      "PUSH START");

  al_draw_text(asteroids.large_font,
      al_map_rgb(WHITE),
      SCREEN_W / 2,
      SCREEN_H - 100,
      ALLEGRO_ALIGN_CENTRE,
      "1 COIN  |  PLAY");
}

static void
start(void)
{
  asteroids.level = level_next(asteroids.level);
}

static void
keydown(ALLEGRO_EVENT ev, bool *keys)
{
  switch(ev.keyboard.keycode) {
    case ALLEGRO_KEY_S:
      keys[KEY_S] = true;
      break;
    case ALLEGRO_KEY_UP:
      keys[KEY_UP] = true;
      break;
    case ALLEGRO_KEY_LEFT:
      keys[KEY_LEFT] = true;
      break;
    case ALLEGRO_KEY_RIGHT:
      keys[KEY_RIGHT] = true;
      break;
    case ALLEGRO_KEY_SPACE:
      keys[KEY_SPACE] = true;
      break;
    case ALLEGRO_KEY_LCTRL:
      keys[KEY_LCONTROL] = true;
      break;
  }
}

static void
keyup(ALLEGRO_EVENT ev, bool *keys)
{
  switch(ev.keyboard.keycode) {
    case ALLEGRO_KEY_S:
      keys[KEY_S] = false;
      break;
    case ALLEGRO_KEY_UP:
      keys[KEY_UP] = false;
      break;
    case ALLEGRO_KEY_LEFT:
      keys[KEY_LEFT] = false;
      break;
    case ALLEGRO_KEY_RIGHT:
      keys[KEY_RIGHT] = false;
      break;
    case ALLEGRO_KEY_SPACE:
      keys[KEY_SPACE] = false;
      break;
    case ALLEGRO_KEY_LCTRL:
      keys[KEY_LCONTROL] = false;
      break;
    case ALLEGRO_KEY_ESCAPE:
      keys[KEY_ESCAPE] = true;
      break;
  }
}

int
main(void)
{
  asteroids.lives         = START_LIVES;
  asteroids.display       = NULL;
  asteroids.timer         = NULL;
  asteroids.event_queue   = NULL;
  SHIP *ship;

  bool redraw = true;
  bool quit   = false;
  bool key[7] = { false };

  seed_rand();
  atexit(shutdown);

  if(!init())
    exit(EXIT_FAILURE);

  if((asteroids.high_score = get_config_value("high_score")) == NULL)
    asteroids.high_score = "0";

  asteroids.level = level_create(0, 0);

  if(!(ship = ship_create()))
    exit(EXIT_FAILURE);

  al_flip_display();
  al_start_timer(asteroids.timer);

  while(!quit) {
    ALLEGRO_EVENT ev;
    al_wait_for_event(asteroids.event_queue, &ev);

    if(ev.type == ALLEGRO_EVENT_TIMER) {
      /* start game */
      if(asteroids.level->number == 0 && key[KEY_S]) {
        start();
        continue;
      }

      /* are we out of asteroids to destroy? */
      if(list_length(asteroids.level->asteroids) == 0)
        asteroids.level = level_next(asteroids.level);

      /* update objects */
      ship = ship_update(ship, key, asteroids.timer);
      explosions_update();
      asteroids.level = level_update(asteroids.level, ship, key, asteroids.timer);

      /* ship->asteroid collisions. */
      check_ship_asteroid_collisions(ship, asteroids.level->asteroids);

      /* ship[missile] -> asteroid collisions. */
      check_ship_missile_asteroid_collisions(asteroids.level, ship);

      /* ship[missile] -> saucer collisions. */
      check_ship_missile_saucer_collisions(asteroids.level, ship);

      /* saucer[missile] -> ship collisions. */
      check_saucer_missile_ship_collisions(asteroids.level, ship);

      /* saucer[missile] -> asteroid collisions. */
      check_saucer_missile_asteroids_collisions(asteroids.level);

      /* saucer->asteroid collisions. */
      check_saucer_asteroid_collisions(asteroids.level);

      redraw = true;
    } else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
      keydown(ev, key);
      quit = key[KEY_ESCAPE];
    } else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
      keyup(ev, key);
      ship->fire_debounce  = key[KEY_SPACE];
      ship->hyper_debounce = key[KEY_LCONTROL];
    }

    if(redraw && al_is_event_queue_empty(asteroids.event_queue)) {
      redraw = false;
      al_clear_to_color(al_map_rgb(BLACK));

      level_draw(asteroids.level);
      draw_lives();
      draw_high_score();
      animation_draw_list(asteroids.explosions);

      if(asteroids.level->number == 0) {
        draw_home();
        al_flip_display();
        continue;
      }

      if(asteroids.lives > 0) {
        ship_draw(ship, key[KEY_UP]);
        missile_draw_list(ship->missiles);
      } else {
        if(ship->explosion)
          ship_draw(ship, false);
        draw_gameover();
      }

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

  LIST *head = list_first(asteroids.level->asteroids);
  while(head != NULL) {
    ASTEROID *rock = (ASTEROID *) head->data;
    asteroid_free(rock);
    head = head->next;
  }
  ship_free(ship);

  al_destroy_bitmap(asteroids.lives_sprite);
  ship_shutdown();
  missile_shutdown();
  asteroid_shutdown();
  explosion_shutdown();

  al_uninstall_keyboard();

  exit(EXIT_SUCCESS);
}
