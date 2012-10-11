#ifndef asteroids_missile_H
#define asteroids_missile_H

#define MISSILE_TTL   1

void missile_free(MISSILE *missile);
void missile_draw(MISSILE *missile);
void update_missile(MISSILE *missile, ALLEGRO_TIMER *timer);
MISSILE *create_missile(void);

#endif /* asteroids_missile_H */
