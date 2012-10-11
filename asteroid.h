#ifndef asteroids_asteroid_H
#define asteroids_asteroid_H

ASTEROID *create_asteroid(uint8_t size);
void asteroid_draw(ASTEROID *asteroid);
void asteroid_free(ASTEROID *asteroid);
void asteroid_update(ASTEROID *asteroid);

#endif /* asteroids_asteroid_H */
