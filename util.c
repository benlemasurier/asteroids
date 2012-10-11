#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

float
deg2rad(float deg)
{
  return deg * (M_PI / 180);
}

float
rand_f(float low, float high)
{
  return low + (float) rand() / ((float) RAND_MAX / (high - low));
}

void
seed_rand(void)
{
  struct timeval t;
  gettimeofday(&t, NULL);
  srand(t.tv_usec * t.tv_sec);
}
