#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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
