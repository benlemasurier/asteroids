#ifndef asteroids_animation_H
#define asteroids_animation_H

void animation_free(ANIMATION *animation);
ANIMATION *new_animation(ALLEGRO_BITMAP **sprites, size_t n_frames);
void draw_animation(ANIMATION *animation);
void update_animation(ANIMATION *animation);

#endif /* asteroids_animation_H */
