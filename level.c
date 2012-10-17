#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>

#include "asteroid.h"
#include "asteroids.h"
#include "saucer.h"
#include "list.h"
#include "level.h"

LEVEL *
create_level(uint8_t level_number)
{
  LEVEL *level = malloc(sizeof(LEVEL));
  level->asteroids = NULL;
  level->saucer    = NULL;
  level->number    = level_number;

  for(int i = 0; i < level_number + START_ASTEROIDS; i++)
    level->asteroids = list_append(level->asteroids, create_asteroid(ASTEROID_LARGE));

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
  if(level->saucer)
    saucer_update(level->saucer, ship, timer);

  return;
}
