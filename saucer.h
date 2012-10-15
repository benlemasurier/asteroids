#ifndef asteroids_saucer_H
#define asteroids_saucer_H

enum {
  SAUCER_SMALL,
  SAUCER_LARGE
};

typedef struct saucer_t {
  uint8_t size;
  unsigned int points;

  float angle;
  VECTOR *position;
  VECTOR *velocity;

  ALLEGRO_BITMAP *sprite;
} SAUCER;

bool saucer_init(void);
SAUCER *saucer_new(uint8_t size);
void saucer_free(SAUCER *saucer);
void saucer_update(SAUCER *saucer);

#endif /* asteroids_saucer_H */
