#ifndef asteroids_explosion_H
#define asteroids_explosion_H

void new_explosion(VECTOR *position);
bool explosion_init(void);
void remove_explosion(ANIMATION *explosion);
void explosions_update(void);
void explosions_draw(void);

#endif /* asteroids_explosion_H */
