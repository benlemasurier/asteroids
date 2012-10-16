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
rad2deg(float rad)
{
  return rad * (180 / M_PI);
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

float
get_angle(float p1x, float p1y, float p2x, float p2y)
{
  return atan2f(p2y - p1y, p2x - p1x);
}
