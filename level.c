#include <stdio.h>
#include <stdlib.h>

#include "asteroid.h"
#include "asteroids.h"
#include "level.h"

LEVEL *
create_level(int n_asteroids)
{
  LEVEL *level = malloc(sizeof(LEVEL));
  level->asteroids = NULL;

  for(int i = 0; i < n_asteroids; i++)
    level->asteroids = list_append(level->asteroids, create_asteroid(ASTEROID_LARGE));

  return level;
}
