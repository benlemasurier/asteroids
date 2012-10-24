#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#include "asteroid.h"
#include "asteroids.h"
#include "saucer.h"
#include "list.h"
#include "level.h"

ALLEGRO_FONT *large_font;

static void
draw_score(unsigned long int score)
{
  char score_s[20];
  sprintf(score_s, "%02lu", score);

  al_draw_text(large_font,
      al_map_rgb(WHITE),
      SCORE_X,
      SCORE_Y,
      ALLEGRO_ALIGN_RIGHT,
      score_s);
}

LEVEL *
level_create(uint8_t level_number, unsigned long int score)
{
  LEVEL *level = malloc(sizeof(LEVEL));
  level->asteroids    = NULL;
  level->saucer       = NULL;
  level->number       = level_number;
  level->saucer_seen  = 0;
  level->playable     = true;
  level->score        = score;

  uint8_t n_create;
  switch(level_number) {
    case 0: /* home screen */
      level->playable = false;
      n_create = 6;
      break;
    case 1:
      n_create = 4;
      break;
    case 2:
      n_create = 6;
      break;
    case 3:
      n_create = 8;
      break;
    case 4:
      n_create = 10;
      break;
    case 5:
      n_create = 12;
      break;
    default:
      n_create = MAX_START_ASTEROIDS;
  }

  for(int i = 0; i < n_create; i++)
    level->asteroids = list_append(level->asteroids, asteroid_create(ASTEROID_LARGE));

  return level;
}

void
level_draw(LEVEL *level)
{
  draw_score(level->score);
  asteroid_draw_list(level->asteroids);

  if(level->saucer)
    saucer_draw(level->saucer);
}

void
level_free(LEVEL *level)
{
  LIST *head = list_first(level->asteroids);
  while(head) {
    asteroid_free((ASTEROID *) head->data);
    head = head->next;
  }

  list_free(level->asteroids);

  free(level);
}

bool
level_init(void)
{
  large_font = al_load_ttf_font("data/vectorb.ttf", 24, 0);

  return true;
}

LEVEL *
level_next(LEVEL *level)
{
  LEVEL *old = level;
  level = level_create(old->number + 1, old->score);
  level_free(old);
  al_rest(2.0);

  return level;
}

void
level_update(LEVEL *level, SHIP *ship, bool keys[], ALLEGRO_TIMER *timer)
{
  int64_t time_count = al_get_timer_count(timer);
  uint8_t next_saucer = abs(level->number + (-LEVEL_MAX)) * 1.3;

  /* is it time to introduce a new saucer? */
  if(!level->saucer)
    if(seconds_elapsed(time_count - level->saucer_seen) > next_saucer)
      level->saucer = saucer_new(SAUCER_LARGE, level->number, timer);

  if(level->saucer) {
    level->saucer = saucer_update(level->saucer, ship, timer);
    level->saucer_seen = time_count;

    /* saucer missiles */
    if(!level->saucer->missile->active)
      saucer_fire(level->saucer, ship, timer);
  }

  asteroid_update_list(level->asteroids);

  return;
}
