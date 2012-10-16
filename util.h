#ifndef asteroids_util_H
#define asteroids_util_H

float deg2rad(float deg);
float rad2deg(float rad);
float rand_f(float low, float high);
void  seed_rand(void);
float get_angle(float px1, float py1, float px2, float py2);

#endif /* asteroids_util_H */
