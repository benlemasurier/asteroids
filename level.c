#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>

#include "asteroid.h"
#include "asteroids.h"
#include "saucer.h"
#include "list.h"
#include "level.h"

LEVEL *
level_create(uint8_t level_number)
{
  LEVEL *level = malloc(sizeof(LEVEL));
  level->asteroids    = NULL;
  level->saucer       = NULL;
  level->number       = level_number;
  level->saucer_seen  = 0;

  for(int i = 0; i < level_number + START_ASTEROIDS; i++)
    level->asteroids = list_append(level->asteroids, asteroid_create(ASTEROID_LARGE));

  return level;
}

void
level_free(LEVEL *level)
{
  LIST *head = list_first(level->asteroids);
  while(head) {
    asteroid_free((ASTEROID *) head->data);
    head = head->next;
  }

  /* FIXME: need list_free() */

  free(level);
}

void
level_update(LEVEL *level, SHIP *ship, ALLEGRO_TIMER *timer)
{
  int64_t time_count = al_get_timer_count(timer);
  uint8_t next_saucer = abs(level->number + (-LEVEL_MAX)) * 2;

  /* is it time to introduce a new saucer? */
  if(!level->saucer)
    if(seconds_elapsed(time_count - level->saucer_seen) > next_saucer)
      level->saucer = saucer_new(SAUCER_LARGE, level->number);

  if(level->saucer) {
    saucer_update(level->saucer, ship, timer);
    level->saucer_seen = time_count;
  }

  return;
}
