#ifndef asteroids_H
#define asteroids_H

typedef struct vector_t {
  float x;
  float y;
} VECTOR;

typedef struct animation_t {
  uint8_t width;
  uint8_t height;
  VECTOR *position;

  size_t  n_frames;
  size_t  current_frame;

  /* slowdown factor, play each frame `slowdown` times */
  uint8_t slowdown;

  /* how many times has the current frame been played? */
  uint8_t frame_played;

  ALLEGRO_BITMAP **sprites;
} ANIMATION;

typedef struct missile_t {
  int width;
  int height;
  bool active;
  int64_t time;

  float angle;
  VECTOR *position;
  VECTOR *velocity;

  ALLEGRO_BITMAP *sprite;
} MISSILE;

typedef struct ship_t {
  int width;
  int height;
  bool thrust_visible;
  bool fire_debounce;
  bool hyper_debounce;

  MISSILE **missiles;

  float angle;
  VECTOR *position;
  VECTOR *velocity;

  ANIMATION *explosion;

  ALLEGRO_BITMAP *sprite;
  ALLEGRO_BITMAP *thrust_sprite;
} SHIP;

typedef struct asteroid_t {
  int width;
  int height;
  float angle;
  uint8_t size;
  uint8_t points;

  VECTOR *position;
  VECTOR *velocity;

  ALLEGRO_BITMAP *sprite;
} ASTEROID;

typedef struct level_t {
  int n_asteroids;
  ASTEROID **asteroids;
} LEVEL;

static void shutdown(void);
static void rotate_ship(SHIP *ship, float deg);
static void wrap_position(VECTOR *position);
static float deg2rad(float deg);
static float rand_f(float low, float high);
static bool preload_asteroid_sprites(void);
static bool preload_ship_sprites(void);
static bool init(void);
static MISSILE *create_missile(void);
static SHIP *create_ship(void);
static ALLEGRO_BITMAP *load_asteroid_sprite(uint8_t size, float angle);
static ASTEROID *create_asteroid(uint8_t size);
static void free_asteroid(ASTEROID *asteroid);
static void free_animation(ANIMATION *animation);
static LEVEL *create_level(int n_asteroids);
static ANIMATION *new_animation(ALLEGRO_BITMAP **sprites, size_t n_frames);
static void new_explosion(VECTOR *position);
static void ship_explode(SHIP *ship);
static void remove_explosion(ANIMATION *explosion);
static void launch_missile(SHIP *ship);
static void free_ship(SHIP *ship);
static void free_missile(MISSILE *missile);
static void hyperspace(SHIP *ship);
static void accelerate(SHIP *ship);
static void drag(SHIP *ship);
static void draw_animation(ANIMATION *animation);
static void draw_ship(SHIP *ship, bool thrusting);
static void draw_asteroid(ASTEROID *asteroid);
static void draw_missile(MISSILE *missile);
static void draw_score(void);
static void draw_high_score(void);
static void draw_lives(void);
static bool collision(float b1_x, float b1_y, int b1_w, int b1_h, float b2_x, float b2_y, int b2_w, int b2_h);
static bool asteroid_collision(SHIP *ship, ASTEROID *asteroid);
static bool missile_collision(MISSILE *missile, ASTEROID *asteroid);
static void explode_asteroid(ASTEROID *asteroid);
static void missile_explode_asteroid(MISSILE *missile, ASTEROID *asteroid);
static void update_animation(ANIMATION *animation);
static void update_ship(SHIP *ship);
static void update_asteroid(ASTEROID *asteroid);
static void update_missile(MISSILE *missile);
static void seed_rand(void);

#endif /* asteroids_H */
