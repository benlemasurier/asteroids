#include <stdio.h>
#include <stdlib.h>

#include "level.h"
#include "asteroids.h"

LEVEL *
create_level(int n_asteroids)
{
  LEVEL *level = malloc(sizeof(LEVEL));

  level->n_asteroids = n_asteroids;
  level->asteroids = malloc(sizeof(ASTEROID *) * n_asteroids);
  for(int i = 0; i < n_asteroids; i++)
    level->asteroids[i] = create_asteroid(ASTEROID_LARGE);

  return level;
}
