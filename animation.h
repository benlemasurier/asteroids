#ifndef asteroids_animation_H
#define asteroids_animation_H

void animation_draw(ANIMATION *animation);
void animation_free(ANIMATION *animation);
ANIMATION *animation_new(ALLEGRO_BITMAP **sprites, size_t n_frames);
void animation_update(ANIMATION *animation);

#endif /* asteroids_animation_H */
