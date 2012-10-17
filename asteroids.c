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

enum CONTROLS {
  KEY_UP,      /* thrust */
  KEY_LEFT,    /* rotate left */
  KEY_RIGHT,   /* rotate right */
  KEY_SPACE,   /* fire ze missiles */
  KEY_LCONTROL /* HYPERSPACE! */
};

static struct asteroids {
  unsigned long int score;
  unsigned long int high_score;

  uint8_t current_level;

  int lives;
  LEVEL *level;

  uint8_t   n_explosions;
  ANIMATION **explosions;

  ALLEGRO_FONT    *small_font;
  ALLEGRO_FONT    *large_font;
  ALLEGRO_TIMER   *timer;
  ALLEGRO_DISPLAY *display;
  ALLEGRO_EVENT_QUEUE *event_queue;

  ALLEGRO_BITMAP *lives_sprite;
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

static void
draw_score(void)
{
  char score[20];
  sprintf(score, "%02lu", asteroids.score);

  al_draw_text(asteroids.large_font,
      al_map_rgb(WHITE),
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
      al_map_rgb(WHITE),
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

static void
draw_asteroids(LIST *rocks)
{
  LIST *head = list_first(rocks);
  while(head != NULL) {
    ASTEROID *rock = (ASTEROID *) head->data;
    asteroid_draw(rock);

    head = head->next;
  }
}

static void
draw_missiles(LIST *missiles)
{
  LIST *head = list_first(missiles);
  while(head) {
    MISSILE *m = (MISSILE *) head->data;

    if(m->active)
      missile_draw(m);

    head = head->next;
  }
}

static void
explosions_draw(void)
{
  for(int i = 0; i < asteroids.n_explosions; i++)
    animation_draw(asteroids.explosions[i]);
}

static void
new_explosion(VECTOR *position)
{
  ANIMATION *explosion = explosion_create(position);

  asteroids.n_explosions++;
  asteroids.explosions = (ANIMATION **) realloc(asteroids.explosions, sizeof(ANIMATION *) * asteroids.n_explosions);
  asteroids.explosions[asteroids.n_explosions - 1] = explosion;
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

  animation_free(explosion);
}

static void
explosions_update(void)
{
  for(int i = 0; i < asteroids.n_explosions; i++)
    if(asteroids.explosions[i]->current_frame < asteroids.explosions[i]->n_frames)
      animation_update(asteroids.explosions[i]);
    else
      remove_explosion(asteroids.explosions[i]);
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
  asteroids.lives_sprite = al_load_bitmap("data/sprites/ship/ship.png");
  if(!asteroids.lives_sprite) {
    fprintf(stderr, "failed to load lives sprite.\n");
    return false;
  }

  /* sprite preloading */
  if(!ship_init())
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
asteroid_ship_collision(SHIP *ship, ASTEROID *asteroid)
{
  float ship_x = ship->position->x - (ship->width  / 2);
  float ship_y = ship->position->y - (ship->height / 2);
  float rock_x = asteroid->position->x - (asteroid->width  / 2);
  float rock_y = asteroid->position->y - (asteroid->height / 2);

  return collision(ship_x, ship_y, ship->width, ship->height,
                   rock_x, rock_y, asteroid->width, asteroid->height);
}

static bool
asteroid_saucer_collision(SAUCER *saucer, ASTEROID *asteroid)
{
  float saucer_x = saucer->position->x - (saucer->width  / 2);
  float saucer_y = saucer->position->y - (saucer->height / 2);
  float rock_x = asteroid->position->x - (asteroid->width  / 2);
  float rock_y = asteroid->position->y - (asteroid->height / 2);

  return collision(saucer_x, saucer_y, saucer->width, saucer->height,
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

  if(asteroid->size > ASTEROID_SMALL) {
    ASTEROID *tmp = create_asteroid(asteroid->size - 1);
    tmp->position->x = asteroid->position->x;
    tmp->position->y = asteroid->position->y;
    level->asteroids = list_append(level->asteroids, tmp);

    tmp = create_asteroid(asteroid->size - 1);
    tmp->position->x = asteroid->position->x;
    tmp->position->y = asteroid->position->y;
    level->asteroids = list_append(level->asteroids, tmp);
  }

  asteroid_free(asteroid);
}

static void
asteroids_update(LIST *rocks)
{
  LIST *head = list_first(rocks);
  while(head) {
    ASTEROID *rock = (ASTEROID *) head->data;
    asteroid_update(rock);

    head = head->next;
  }
}

static void
missile_explode_asteroid(MISSILE *missile, ASTEROID *asteroid)
{
  missile->active = false;
  asteroids.score += asteroid->points;

  new_explosion(missile->position);
  explode_asteroid(asteroid);
}

static int64_t
seconds_elapsed(ALLEGRO_TIMER *timer)
{
  /* FIXME: this is correct at 60FPS, but probably
     not how timers actually work */
  return(al_get_timer_count(timer) / FPS);
}

int
main(void)
{
  asteroids.score         = 0;
  asteroids.lives         = START_LIVES;
  asteroids.display       = NULL;
  asteroids.timer         = NULL;
  asteroids.event_queue   = NULL;
  asteroids.current_level = 1;
  SHIP *ship;

  bool redraw = true;
  bool quit   = false;
  bool key[5] = { false };

  seed_rand();
  atexit(shutdown);

  if(!init())
    exit(EXIT_FAILURE);

  if(!(ship = ship_create()))
    exit(EXIT_FAILURE);

  asteroids.level = create_level(asteroids.current_level);

  al_flip_display();
  al_start_timer(asteroids.timer);

  while(!quit) {
    ALLEGRO_EVENT ev;
    LIST *head = NULL;
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

      /* TESTING: saucers */
      if(seconds_elapsed(asteroids.timer) == 10) {
        if(!asteroids.level->saucer) {
          asteroids.level->saucer = saucer_new(SAUCER_LARGE, asteroids.level->number);
        }
      }

      /* are we out of asteroids to destroy? */
      if(list_length(asteroids.level->asteroids) == 0) {
        asteroids.current_level += 1; /* TODO: levels have a ceiling, what is it? */
        LEVEL *old = asteroids.level;
        asteroids.level = create_level(asteroids.current_level);
        level_free(old);
        al_rest(2.0);
      }

      /* update positions */
      ship = ship_update(ship, asteroids.timer);
      asteroids_update(asteroids.level->asteroids);
      explosions_update();
      level_update(asteroids.level, ship, asteroids.timer);

      /* ship->asteroid collisions. */
      if(!ship->explosion) {
        head = list_first(asteroids.level->asteroids);
        while(head) {
          ASTEROID *asteroid = (ASTEROID *) head->data;

          if(asteroid_ship_collision(ship, asteroid)) {
            asteroids.score += asteroid->points;
            explode_asteroid(asteroid);

            if(ship_explode(ship))
              asteroids.lives--;

            head = list_first(asteroids.level->asteroids);
          }

          head = head->next;
        }
      }

      /* ship[missile] -> asteroid collisions. FIXME: who made this mess? */
      LIST *missile_head = list_first(ship->missiles);
      while(missile_head) {
        MISSILE *m = (MISSILE *) missile_head->data;

        if(m->active) {
          LIST *rocks = list_first(asteroids.level->asteroids);
          while(rocks) {
            ASTEROID *a = (ASTEROID *) rocks->data;

            if(missile_collision(m, a)) {
              missile_explode_asteroid(m, a);
              rocks = list_first(asteroids.level->asteroids);
              continue;
            }

            rocks = rocks->next;
          }
        }

        missile_head = missile_head->next;
      }

      /* saucer[missile] -> ship collisions. */
      if(asteroids.level->saucer) {
        if(asteroids.level->saucer->missile) {
          MISSILE *m = asteroids.level->saucer->missile;
          if(saucer_missile_ship_collision(m, ship)) {
            asteroids.lives--;
            ship_explode(ship);
            /* TODO: saucer_free(asteroids.level->saucer); */
          }
        }
      }

      /* saucer->asteroid collisions. */
      if(asteroids.level->saucer) {
        head = list_first(asteroids.level->asteroids);
        while(head && asteroids.level->saucer) {
          ASTEROID *asteroid = (ASTEROID *) head->data;

          if(asteroid_saucer_collision(asteroids.level->saucer, asteroid)) {
            new_explosion(asteroid->position);
            explode_asteroid(asteroid);
            saucer_free(asteroids.level->saucer);
            asteroids.level->saucer = NULL;
          }

          head = head->next;
        }
      }

      /* saucer missiles */
      if(asteroids.level->saucer)
        saucer_fire(asteroids.level->saucer, ship, asteroids.timer);

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
      al_clear_to_color(al_map_rgb(BLACK));

      draw_score();
      draw_lives();
      draw_high_score();
      ship_draw(ship, key[KEY_UP]);
      draw_missiles(ship->missiles);
      draw_asteroids(asteroids.level->asteroids);
      explosions_draw();
      if(asteroids.level->saucer)
        saucer_draw(asteroids.level->saucer);

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
  asteroid_shutdown();

  al_uninstall_keyboard();

  exit(EXIT_SUCCESS);
}
