#include "collision.h"
#include "forces.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "star.h"
#include "state.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct info {
  int body_type;
} info_t;

// window constants
const vector_t MIN = {.x = 0.0, .y = 0.0};
const vector_t MAX = {.x = 1000.0, .y = 500.0};
const vector_t CENTER = {.x = 500.0, .y = 250.0};

// game constants
const double BRICK_SEP = 3.25;
const double BRICK_WIDTH = 120.0;
const double BRICK_HEIGHT = 40.0;
const double BALL_RADIUS = 15.0;
const double BUFFER = 25.0;
const double WALL_BUFF = 5.0;
const double BALL_BUFF = 30.0;
const double X_INIT_BRICK = BRICK_SEP + (BRICK_WIDTH / 2) + 5.0;
const double Y_INIT_BRICK = 500.0 - (BRICK_HEIGHT / 2) - BRICK_SEP - 5.0;
const size_t NUM_BRICK = 24;
const size_t NUM_BRICK_LINE = 8;
const double EXPLOSION_RAD_X = (BRICK_WIDTH / 2.0) + 65;
const double EXPLOSION_RAD_Y = 50.0;
const int GOOD_SEED = 69;

// type
const int PLAY_TYPE = 0;
const int BRICK_TYPE = 2;
const int BALL_TYPE = 1;
const int SPECIAL_TYPE = 3;

// color constants
const rgb_color_t COLOR_PLAY = (rgb_color_t){0.0, 0.0, 1.0};
const rgb_color_t COLOR_WALL = (rgb_color_t){1.0, 1.0, 1.0};
const rgb_color_t SPECIAL_COLOR = (rgb_color_t){0.0, 0.0, 0.0};
const rgb_color_t BRICK_COLORS[8] = {
    (rgb_color_t){1.0, 0.0, 1.0}, (rgb_color_t){1.0, 0.0, 0.0},
    (rgb_color_t){1.0, 1.0, 0.0}, (rgb_color_t){0.5, 1.0, 0.0},
    (rgb_color_t){0.0, 1.0, 0.0}, (rgb_color_t){0.0, 1.0, 1.0},
    (rgb_color_t){0.0, 0.0, 1.0}, (rgb_color_t){0.5, 0.0, 1.0}};

// physical constants
const size_t NUM_CIRC_POINTS = 20;
const double MASS = INFINITY;
const double PLAYER_MASS = 2.0;
const size_t NUM_RECT_POINTS = 4;
const vector_t ZERO_VEC = {.x = 0.0, .y = 0.0};
const double ELASTICITY = 1.0;
const vector_t BALL_VEL = (vector_t){200.0, 300.0};
const double NORM_VELOCITY = 500.0;

// list constants
const size_t PLAY_INDEX = 0;
const size_t BALL_INDEX = 1;
const size_t BRICK_START_IND = 6;

typedef struct scene_tuple {
  scene_t *scene;
  double time;
  double update_time;
} scene_tuple_t;

typedef struct state {
  scene_t *scene;
  scene_tuple_t *scene_tup;
  double last_time;
  double time;
} state_t;

void reset_init(state_t *state);

size_t rand_range(size_t min, size_t max) {
  return (rand() % (max - min + 1)) + min;
}

double rand_range_double(double min, double max) {
  srand(GOOD_SEED);
  double random = ((double)rand()) / RAND_MAX;
  double range = (max - min) * random;
  return min + range;
}

body_t *make_brick(double x_in, double y_in, int brick_num) {
  list_t *shape = list_init(NUM_RECT_POINTS, (free_func_t)free);
  vector_t *v = malloc(sizeof(*v));
  *v = (vector_t){(x_in) + (BRICK_WIDTH / 2), y_in + (BRICK_HEIGHT / 2)};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){(x_in) - (BRICK_WIDTH / 2), y_in + (BRICK_HEIGHT / 2)};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){(x_in) - (BRICK_WIDTH / 2), y_in - (BRICK_HEIGHT / 2)};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){(x_in) + (BRICK_WIDTH / 2), y_in - (BRICK_HEIGHT / 2)};
  list_add(shape, v);
  info_t *brick_info = malloc(sizeof(info_t));
  brick_info->body_type = BRICK_TYPE;
  body_t *brick = body_init_with_info(shape, MASS, BRICK_COLORS[brick_num],
                                      brick_info, (free_func_t)free);
  return brick;
}

body_t *make_player() {
  list_t *shape = list_init(NUM_RECT_POINTS, (free_func_t)free);
  vector_t *v = malloc(sizeof(*v));
  *v = (vector_t){(CENTER.x) + (BRICK_WIDTH / 2),
                  BUFFER + BRICK_SEP + (BRICK_HEIGHT / 2)};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){(CENTER.x) - (BRICK_WIDTH / 2),
                  BUFFER + BRICK_SEP + (BRICK_HEIGHT / 2)};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){(CENTER.x) - (BRICK_WIDTH / 2),
                  BUFFER + BRICK_SEP - (BRICK_HEIGHT / 2)};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){(CENTER.x) + (BRICK_WIDTH / 2),
                  BUFFER + BRICK_SEP - (BRICK_HEIGHT / 2)};
  list_add(shape, v);
  info_t *player_info = malloc(sizeof(info_t));
  player_info->body_type = PLAY_TYPE;
  body_t *player = body_init_with_info(shape, MASS, COLOR_PLAY, player_info,
                                       (free_func_t)free);
  return player;
}

body_t *make_horizontal_wall() {
  list_t *shape = list_init(NUM_RECT_POINTS, (free_func_t)free);
  vector_t *v = malloc(sizeof(*v));
  *v = (vector_t){MAX.x, MAX.y};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){MIN.x, MAX.y};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){MIN.x, MAX.y - WALL_BUFF};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){MAX.x, MAX.y - WALL_BUFF};
  list_add(shape, v);
  info_t *player_info = malloc(sizeof(info_t));
  player_info->body_type = PLAY_TYPE;
  body_t *player = body_init_with_info(shape, INFINITY, COLOR_WALL, player_info,
                                       (free_func_t)free);
  return player;
}

body_t *make_vertical_wall() {
  list_t *shape = list_init(NUM_RECT_POINTS, (free_func_t)free);
  vector_t *v = malloc(sizeof(*v));
  *v = (vector_t){MAX.x, MAX.y};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){MAX.x - WALL_BUFF, MAX.y};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){MAX.x - WALL_BUFF, MIN.y};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){MAX.x, MIN.y};
  list_add(shape, v);
  info_t *player_info = malloc(sizeof(info_t));
  player_info->body_type = PLAY_TYPE;
  body_t *player = body_init_with_info(shape, INFINITY, COLOR_WALL, player_info,
                                       (free_func_t)free);
  return player;
}

body_t *make_ball() {
  list_t *shape = list_init(NUM_CIRC_POINTS, (free_func_t)free);
  double curr_angle = 0;
  double vert_angle = (2.0 * M_PI) / NUM_CIRC_POINTS;
  double x = CENTER.x;
  double y = BALL_BUFF + BRICK_HEIGHT;
  for (size_t i = 0; i < NUM_CIRC_POINTS; i++) {
    vector_t *curr_vec = malloc(sizeof(vector_t));
    assert(curr_vec != NULL);
    curr_vec->x = cos(curr_angle) * BALL_RADIUS + x;
    curr_vec->y = sin(curr_angle) * BALL_RADIUS + y;
    curr_angle += vert_angle;
    list_add(shape, curr_vec);
  }
  info_t *ball_info = malloc(sizeof(info_t));
  ball_info->body_type = BALL_TYPE;
  body_t *ball = body_init_with_info(shape, PLAYER_MASS, COLOR_PLAY, ball_info,
                                     (free_func_t)free);
  body_set_velocity(ball, BALL_VEL);
  return ball;
}

scene_t *game_init() {
  scene_t *scene = scene_init();
  scene_add_body(scene, make_player());
  scene_add_body(scene, make_ball());
  scene_add_body(scene, make_horizontal_wall());
  scene_add_body(scene, make_vertical_wall());
  body_t *left_wall = make_vertical_wall();
  vector_t wall_center = body_get_centroid(left_wall);
  wall_center.x -= MAX.x - WALL_BUFF;
  body_set_centroid(left_wall, wall_center);
  body_t *bottom_wall = make_horizontal_wall();
  wall_center = body_get_centroid(bottom_wall);
  wall_center.y -= MAX.y - WALL_BUFF;
  body_set_centroid(bottom_wall, wall_center);
  scene_add_body(scene, left_wall);
  scene_add_body(scene, bottom_wall);

  double x = X_INIT_BRICK;
  double y = Y_INIT_BRICK;
  for (int i = 1; i < NUM_BRICK + 1; i++) {
    scene_add_body(scene, make_brick(x, y, i % NUM_BRICK_LINE));
    x += (BRICK_WIDTH + BRICK_SEP);
    if (i % NUM_BRICK_LINE == 0) {
      y -= (BRICK_HEIGHT + BRICK_SEP);
      x = X_INIT_BRICK;
    }
  }
  return scene;
}

void brick_destruction(body_t *ball, body_t *brick, vector_t axis, void *aux) {
  body_remove(brick);
}

void special_case(body_t *ball, body_t *brick, vector_t axis, void *aux) {
  double r_y = EXPLOSION_RAD_Y;
  double r_x = EXPLOSION_RAD_X;
  vector_t enemy_c = body_get_centroid(brick);
  scene_t *scene = aux;
  for (size_t i = BRICK_START_IND; i < scene_bodies(scene); i++) {
    body_t *curr_body = scene_get_body(scene, i);
    vector_t center = body_get_centroid(curr_body);
    if (!body_is_removed(curr_body)) {
      if ((center.x < enemy_c.x + r_x && center.x > enemy_c.x - r_x) &&
          (center.y < enemy_c.y + r_y && center.y > enemy_c.y - r_y)) {
        body_remove(curr_body);
      }
    }
  }
}

void restart_game(body_t *ball, body_t *wall, vector_t axis, void *aux) {
  state_t *state = aux;
  sdl_clear();
  free(state->scene_tup);
  reset_init(state);
}

void exit_game(body_t *ball, body_t *wall, vector_t axis, void *aux) {
  exit(1);
}

void add_wall_collisions(state_t *state) {
  body_t *ball = scene_get_body(state->scene, 1);
  for (size_t i = 2; i < BRICK_START_IND - 1; i++) {
    body_t *curr_body = scene_get_body(state->scene, i);
    create_physics_collision(state->scene, ELASTICITY, ball, curr_body);
  }
  create_collision(state->scene, ball, scene_get_body(state->scene, 5),
                   restart_game, state, NULL);
}

void add_forces(state_t *state) {
  body_t *ball = scene_get_body(state->scene, 1);
  body_t *paddle = scene_get_body(state->scene, 0);
  add_wall_collisions(state);
  create_physics_collision(state->scene, ELASTICITY, ball, paddle);
  size_t special_ind =
      (size_t)rand_range_double(BRICK_START_IND, scene_bodies(state->scene));
  for (size_t i = BRICK_START_IND; i < scene_bodies(state->scene); i++) {
    body_t *curr_body = scene_get_body(state->scene, i);
    if (!body_is_removed(curr_body)) {
      if (i == special_ind) {
        body_set_color(curr_body, SPECIAL_COLOR);
        create_collision(state->scene, ball, curr_body, special_case,
                         state->scene, NULL);
        create_physics_collision(state->scene, ELASTICITY, ball, curr_body);
      } else {
        create_physics_collision(state->scene, ELASTICITY, ball, curr_body);
        create_collision(state->scene, ball, curr_body, brick_destruction,
                         state->scene, NULL);
      }
    }
  }
}

vector_t in_bounds(vector_t center, double outer_rad) {
  if (center.x + outer_rad > MAX.x - WALL_BUFF - BRICK_SEP) {
    return (vector_t){MAX.x - WALL_BUFF - BRICK_SEP - outer_rad, center.y};
  }
  if (center.x - outer_rad < MIN.x + WALL_BUFF + BRICK_SEP) {
    return (vector_t){MIN.x + WALL_BUFF + BRICK_SEP + outer_rad, center.y};
  }
  return center;
}

void compute_new_position(scene_t *scene, vector_t velocity, double dt) {
  body_t *body = scene_get_body(scene, PLAY_INDEX);
  body_set_velocity(body, velocity);
}

void reset_vel(scene_t *scene) {
  body_t *body = scene_get_body(scene, PLAY_INDEX);
  body_set_velocity(body, ZERO_VEC);
}

void keyHandle(char key, key_event_type_t type, double held_time,
               scene_tuple_t *scene_tup) {
  if (type == KEY_PRESSED) {
    switch (key) {
    case LEFT_ARROW:
      compute_new_position(scene_tup->scene, (vector_t){-NORM_VELOCITY, 0.0},
                           held_time);
      break;
    case RIGHT_ARROW:
      compute_new_position(scene_tup->scene, (vector_t){NORM_VELOCITY, 0.0},
                           held_time);
      break;
    }
  }
  if (type == KEY_RELEASED) {
    switch (key) {
    case LEFT_ARROW:
      reset_vel(scene_tup->scene);
      break;
    case RIGHT_ARROW:
      reset_vel(scene_tup->scene);
      break;
    }
  }
}

void reset_init(state_t *state) {
  state->scene = game_init();
  scene_tuple_t *sc_tup = malloc(sizeof(scene_tuple_t));
  add_forces(state);
  state->scene_tup = sc_tup;
  state->scene_tup->scene = state->scene;
  state->scene_tup->time = 0;
  state->scene_tup->update_time = 0;
  state->last_time = 0;
  state->time = 0;
}

state_t *emscripten_init() {
  sdl_init(MIN, MAX);
  state_t *state = malloc(sizeof(state_t));
  assert(state != NULL);
  state->scene = game_init();
  scene_tuple_t *sc_tup = malloc(sizeof(scene_tuple_t));
  add_forces(state);
  state->scene_tup = sc_tup;
  state->scene_tup->scene = state->scene;
  state->scene_tup->time = 0;
  state->scene_tup->update_time = 0;
  state->last_time = 0;
  state->time = 0;
  return state;
}

void emscripten_main(state_t *state) {
  double dt = time_since_last_tick();
  state->scene_tup->time += dt;
  state->last_time = dt;
  state->time += dt;

  sdl_on_key(keyHandle);
  body_t *body = scene_get_body(state->scene, PLAY_INDEX);
  body_set_centroid(body,
                    in_bounds(body_get_centroid(body), (BRICK_WIDTH / 2.0)));

  if (scene_bodies(state->scene) == BRICK_START_IND) {
    exit(1);
  }
  scene_tick(state->scene, state->last_time);
  sdl_render_scene(state->scene);
}

void emscripten_free(state_t *state) {
  free(state->scene_tup);
  scene_free(state->scene);
  free(state);
}
