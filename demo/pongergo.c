#include "collision.h"
#include "forces.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "star.h"
#include "state.h"
#include <SDL2/SDL_ttf.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct info {
  int body_type;
  double width;
  double height;
  int last_hit;
  int powerup_type;
} info_t;

typedef struct scene_tuple {
  scene_t *scene;
  double time;
  double update_time;
  int score1;
  int score2;
  int game_done;
  int pwr_owner;
  bool pwr_on;
  bool pwr_add;
  double spawn_time;
  double pwr_time;
  int pwr_type;
  char *mess;
  double end_time;
} scene_tuple_t;

typedef struct state {
  scene_t *scene;
  scene_tuple_t *scene_tup;
  double last_time;
  double time;
} state_t;

#define atoa(x) #x

// window constants
const vector_t MIN = {.x = 0.0, .y = 0.0};
const vector_t MAX = {.x = 1000.0, .y = 500.0};
const vector_t CENTER = {.x = 500.0, .y = 250.0};
const int MSG_SIZE = 5;
const double PWR_BUFFER_X = 100.0;
const double PWR_BUFFER_Y = 30.0;

// game constants
const int MAX_SCORE = 21;
const double PLAYER_HEIGHT = 100.0;
const double PLAYER_WIDTH = 10.0;
const double GOAL_HEIGHT = 10.0;
const double GOAL_WIDTH = 50.0;
const double STATIC_GOAL_HEIGHT = 150.0;
const double STATIC_GOAL_WIDTH = 10.0;
const double BALL_RADIUS = 10.0;
const double BUFFER = 75.0;
const double WALL_BUFF = 10.0;
const double BALL_BUFF = 30.0;
const double SPAWN_INTERVAL = 10.0;
const double PWR_TIMER = 5.0;

// type
const int PLAY1_TYPE = 0;
const int PLAY2_TYPE = 1;
const int BALL_TYPE = 2;
const int GOAL1_TYPE = 3;
const int GOAL2_TYPE = 4;
const int STATIC_GOAL1_TYPE = 5;
const int STATIC_GOAL2_TYPE = 6;
const int WALL_TYPE = 7;
const int SPECIAL_TYPE = 8;
const int POWERUP_TYPE1 = 1;
const int POWERUP_TYPE2 = 2;
const int POWERUP_TYPE3 = 3;

// color constants
const rgb_color_t COLOR_PLAY1 = (rgb_color_t){0.70, 0.85, 1.00};
const rgb_color_t COLOR_PLAY2 = (rgb_color_t){0.70, 0.85, 1.00};
const rgb_color_t COLOR_GOAL = (rgb_color_t){1.0, 1.0, 1.0};
const rgb_color_t COLOR_BALL = (rgb_color_t){0.0, 0.0, 0.0};
const rgb_color_t COLOR_WALL = (rgb_color_t){0.0, 0.0, 0.0};
const rgb_color_t SPECIAL_COLOR1 = (rgb_color_t){0.0, 0.5, 0.0};
const rgb_color_t SPECIAL_COLOR2 = (rgb_color_t){0.7, 0.5, 0.3};
const rgb_color_t SPECIAL_COLOR3 = (rgb_color_t){0.1, 0.1, 0.8};

// physical constants
const size_t NUM_CIRC_POINTS = 20;
const double MASS = INFINITY;
const double BALL_MASS = 2.0;
const size_t NUM_RECT_POINTS = 4;
const vector_t ZERO_VEC = {.x = 0.0, .y = 0.0};
const double ELASTICITY = 1.0;
const double NORM_VELOCITY = 750.0;
const double ANG_VELOCITY = 25.0;
const vector_t BALL_VEL = (vector_t){-NORM_VELOCITY / 2, NORM_VELOCITY / 10};
const int BALL_SWITCH_X = 2;
const int BALL_SWITCH_Y = 3;
const double GOAL_SCALAR = 0.5;
const double PLAYER_SCALAR = 1.75;
const double PHYS_BUFFER = 5.0;
const double MAX_ROTATION = M_PI / 3;

// list constants
const size_t PLAY1_INDEX = 0;
const size_t PLAY2_INDEX = 1;
const size_t BALL_INDEX = 2;
const size_t WALL_START_INDEX = 3;
const size_t WALL_END_INDEX = 6;
const size_t GOAL1_INDEX = 7;
const size_t GOAL2_INDEX = 8;
const size_t STATIC_GOAL1_INDEX = 9;
const size_t STATIC_GOAL2_INDEX = 10;

// power_list constants
const size_t POWERUP1_INDEX = 11;
const size_t POWERUP2_INDEX = 12;
const size_t POWERUP3_INDEX = 13;

// fontstants
const int FONT_SIZE = 90;
const int END_SIZE = 1000;

const int16_t DEAD_ZONE = 6000;

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
  info_t *wall_info = malloc(sizeof(info_t));
  wall_info->body_type = WALL_TYPE;
  wall_info->height = WALL_BUFF;
  wall_info->width = MAX.y;
  body_t *wall = body_init_with_info(shape, INFINITY, COLOR_WALL, wall_info,
                                     (free_func_t)free);
  return wall;
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
  info_t *wall_info = malloc(sizeof(info_t));
  wall_info->body_type = WALL_TYPE;
  wall_info->height = MAX.y;
  wall_info->width = WALL_BUFF;
  body_t *wall = body_init_with_info(shape, INFINITY, COLOR_WALL, wall_info,
                                     (free_func_t)free);
  return wall;
}

void make_walls(scene_t *scene) {
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
}

body_t *make_player(vector_t CENTER, double HEIGHT, double WIDTH, int type) {
  list_t *shape = list_init(NUM_RECT_POINTS, (free_func_t)free);
  vector_t *v = malloc(sizeof(*v));
  *v = (vector_t){(CENTER.x) + (WIDTH / 2), CENTER.y + (HEIGHT / 2)};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){(CENTER.x) - (WIDTH / 2), CENTER.y + (HEIGHT / 2)};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){(CENTER.x) - (WIDTH / 2), CENTER.y - (HEIGHT / 2)};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){(CENTER.x) + (WIDTH / 2), CENTER.y - (HEIGHT / 2)};
  list_add(shape, v);
  info_t *player_info = malloc(sizeof(info_t));
  player_info->body_type = type;
  player_info->width = WIDTH;
  player_info->height = HEIGHT;
  rgb_color_t color = COLOR_PLAY1;
  if (type == PLAY2_TYPE) {
    color = COLOR_PLAY2;
  }
  body_t *player =
      body_init_with_info(shape, MASS, color, player_info, (free_func_t)free);
  body_set_max_rotation(player, MAX_ROTATION);
  return player;
}

body_t *make_goal(vector_t CENTER, double HEIGHT, double WIDTH, int type) {
  list_t *shape = list_init(NUM_RECT_POINTS, (free_func_t)free);
  vector_t *v = malloc(sizeof(*v));
  *v = (vector_t){(CENTER.x) + (WIDTH / 2), CENTER.y + (HEIGHT / 2)};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){(CENTER.x) - (WIDTH / 2), CENTER.y + (HEIGHT / 2)};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){(CENTER.x) - (WIDTH / 2), CENTER.y - (HEIGHT / 2)};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){(CENTER.x) + (WIDTH / 2), CENTER.y - (HEIGHT / 2)};
  list_add(shape, v);
  info_t *goal_info = malloc(sizeof(info_t));
  goal_info->body_type = type;
  goal_info->width = WIDTH;
  goal_info->height = HEIGHT;
  body_t *goal = body_init_with_info(shape, MASS, COLOR_GOAL, goal_info,
                                     (free_func_t)free);
  return goal;
}

body_t *make_ball() {
  list_t *shape = list_init(NUM_CIRC_POINTS, (free_func_t)free);
  double curr_angle = 0;
  double vert_angle = (2.0 * M_PI) / NUM_CIRC_POINTS;
  double x = CENTER.x;
  double y = CENTER.y;
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
  ball_info->height = BALL_RADIUS;
  ball_info->width = BALL_RADIUS;
  ball_info->last_hit = -1;
  body_t *ball = body_init_with_info(shape, BALL_MASS, COLOR_BALL, ball_info,
                                     (free_func_t)free);
  body_set_velocity(ball, BALL_VEL);
  body_set_max_velocity(ball, 1000.0);
  return ball;
}

body_t *make_powerup(double x_val, double y_val, int powerup,
                     rgb_color_t color) {
  list_t *shape = list_init(NUM_CIRC_POINTS, (free_func_t)free);
  double curr_angle = 0;
  double vert_angle = (2.0 * M_PI) / NUM_CIRC_POINTS;
  double x = CENTER.x;
  double y = CENTER.y;
  for (size_t i = 0; i < NUM_CIRC_POINTS; i++) {
    vector_t *curr_vec = malloc(sizeof(vector_t));
    assert(curr_vec != NULL);
    curr_vec->x = cos(curr_angle) * (BALL_RADIUS * 2) + x;
    curr_vec->y = sin(curr_angle) * (BALL_RADIUS * 2) + y;
    curr_angle += vert_angle;
    list_add(shape, curr_vec);
  }
  info_t *ball_info = malloc(sizeof(info_t));
  ball_info->body_type = SPECIAL_TYPE;
  ball_info->height = BALL_RADIUS;
  ball_info->width = BALL_RADIUS;
  ball_info->powerup_type = powerup;
  body_t *ball = body_init_with_info(shape, BALL_MASS, color, ball_info,
                                     (free_func_t)free);
  body_set_centroid(ball, (vector_t){x_val, y_val});
  return ball;
}

void exit_game(body_t *ball, body_t *wall, vector_t axis, void *aux) {
  exit(1);
}

void music_wall_handler(body_t *ball, body_t *wall, vector_t axis, void *aux) {
  play_wall_audio();
}

void add_wall_collisions(state_t *state) {
  body_t *ball = scene_get_body(state->scene, BALL_INDEX);
  for (size_t i = WALL_START_INDEX; i <= WALL_END_INDEX; i++) {
    body_t *curr_body = scene_get_body(state->scene, i);
    create_physics_collision(state->scene, ELASTICITY, ball, curr_body);
    create_collision(state->scene, ball, curr_body, music_wall_handler, NULL,
                     NULL);
  }
}

void endgame_init(scene_tuple_t *scene_tup) {
  scene_t *scene = scene_init();
  sdl_render_scene(scene, scene_tup->mess);
}

void reset_ball(body_t *ball, body_t *goal, vector_t axis, void *aux) {
  state_t *state = aux;
  scene_tuple_t *scene_tup = state->scene_tup;
  info_t *info = body_get_info(goal);
  if (info->body_type == GOAL1_TYPE || info->body_type == STATIC_GOAL1_TYPE) {
    scene_tup->score1 += 1;
  } else if (info->body_type == GOAL2_TYPE ||
             info->body_type == STATIC_GOAL2_TYPE) {
    scene_tup->score2 += 1;
  }
  snprintf(scene_tup->mess,
           (sizeof(scene_tup->score2) + sizeof(scene_tup->score1)) + MSG_SIZE,
           "%02d || %02d", scene_tup->score2, scene_tup->score1);
  body_set_centroid(ball, (vector_t){CENTER.x, CENTER.y});
  body_set_impulse(ball, VEC_ZERO);
  vector_t ball_vel = BALL_VEL;
  if ((scene_tup->score1 + scene_tup->score2) % BALL_SWITCH_X == 0) {
    ball_vel.x *= -1.0;
  }
  if ((scene_tup->score1 + scene_tup->score2) % BALL_SWITCH_Y == 0) {
    ball_vel.y *= -1.0;
  }
  body_set_velocity(ball, ball_vel);
}

void powerup_handler(body_t *ball, body_t *powerup, vector_t axis, void *aux) {
  scene_tuple_t *scene_tup = aux;
  scene_tup->pwr_on = true;
  scene_tup->pwr_time = scene_tup->time;
  scene_t *scene = scene_tup->scene;
  info_t *info = body_get_info(ball);
  info_t *power_info = body_get_info(powerup);
  play_powerup_audio();
  if (info->last_hit == PLAY1_TYPE) {
    scene_tup->pwr_owner = PLAY1_TYPE;
    body_t *player = scene_get_body(scene, PLAY1_INDEX);
    body_t *goal = scene_get_body(scene, STATIC_GOAL1_INDEX);
    if (power_info->powerup_type == POWERUP_TYPE1) {
      body_y_scale(player, PLAYER_SCALAR);
    }
    if (power_info->powerup_type == POWERUP_TYPE2) {
      body_y_scale(goal, GOAL_SCALAR);
    }
    if (power_info->powerup_type == POWERUP_TYPE3) {
      scene_tup->score2 += 2;
      snprintf(scene_tup->mess,
               (sizeof(scene_tup->score2) + sizeof(scene_tup->score1)) +
                   MSG_SIZE,
               "%02d || %02d", scene_tup->score2, scene_tup->score1);
    }
    body_remove(powerup);
  } else if (info->last_hit == PLAY2_TYPE) {
    scene_tup->pwr_owner = PLAY2_TYPE;
    body_t *player = scene_get_body(scene, PLAY2_INDEX);
    body_t *goal = scene_get_body(scene, STATIC_GOAL2_INDEX);
    if (power_info->powerup_type == POWERUP_TYPE1) {
      body_y_scale(player, PLAYER_SCALAR);
    }
    if (power_info->powerup_type == POWERUP_TYPE2) {
      body_y_scale(goal, GOAL_SCALAR);
    }
    if (power_info->powerup_type == POWERUP_TYPE3) {
      scene_tup->score1 += 2;
      snprintf(scene_tup->mess,
               (sizeof(scene_tup->score2) + sizeof(scene_tup->score1)) +
                   MSG_SIZE,
               "%02d || %02d", scene_tup->score2, scene_tup->score1);
    }
    body_remove(powerup);
  }
}

void last_collision_handler(body_t *ball, body_t *player, vector_t axis,
                            void *aux) {
  info_t *play_info = body_get_info(player);
  info_t *ball_info = body_get_info(ball);
  if (play_info->body_type == PLAY1_TYPE) {
    ball_info->last_hit = PLAY1_TYPE;
  }
  if (play_info->body_type == PLAY2_TYPE) {
    ball_info->last_hit = PLAY2_TYPE;
  }
}

void music_collision_handler(body_t *ball, body_t *player, vector_t axis,
                             void *aux) {
  play_audio();
}

void add_forces(state_t *state) {
  body_t *play1 = scene_get_body(state->scene, PLAY1_INDEX);
  body_t *play2 = scene_get_body(state->scene, PLAY2_INDEX);
  body_t *goal1 = scene_get_body(state->scene, GOAL1_INDEX);
  body_t *goal2 = scene_get_body(state->scene, GOAL2_INDEX);
  body_t *static_goal1 = scene_get_body(state->scene, STATIC_GOAL1_INDEX);
  body_t *static_goal2 = scene_get_body(state->scene, STATIC_GOAL2_INDEX);

  body_t *ball = scene_get_body(state->scene, BALL_INDEX);
  add_wall_collisions(state);

  create_angular_collision(state->scene, ELASTICITY, ball, play1);
  create_collision(state->scene, ball, play1, music_collision_handler, state,
                   NULL);
  create_angular_collision(state->scene, ELASTICITY, ball, play2);
  create_collision(state->scene, ball, play2, music_collision_handler, state,
                   NULL);

  create_collision(state->scene, ball, goal1, reset_ball, state, NULL);
  create_collision(state->scene, ball, goal2, reset_ball, state, NULL);
  create_collision(state->scene, ball, static_goal1, reset_ball, state, NULL);
  create_collision(state->scene, ball, static_goal2, reset_ball, state, NULL);

  create_collision(state->scene, ball, play1, last_collision_handler, state,
                   NULL);
  create_collision(state->scene, ball, play2, last_collision_handler, state,
                   NULL);
}

vector_t in_bounds(vector_t center, double outer_rad, size_t play_type) {
  if (play_type == GOAL2_TYPE) {
    if (center.x + outer_rad > MAX.x - WALL_BUFF - BUFFER - PHYS_BUFFER) {
      double x = MAX.x - WALL_BUFF - BUFFER - PHYS_BUFFER - outer_rad;
      return (vector_t){x, center.y};
    }
    if (center.x - outer_rad < (MAX.x / 2) + BUFFER + PHYS_BUFFER) {
      double x = (MAX.x / 2) + BUFFER + PHYS_BUFFER + outer_rad;
      return (vector_t){x, center.y};
    }
  } else if (play_type == GOAL1_TYPE) {
    if (center.x + outer_rad > (MAX.x / 2) - BUFFER - PHYS_BUFFER) {
      double x = (MAX.x / 2) - BUFFER - PHYS_BUFFER - outer_rad;
      return (vector_t){x, center.y};
    }
    if (center.x - outer_rad < MIN.x + WALL_BUFF + BUFFER + PHYS_BUFFER) {
      double x = MIN.x + WALL_BUFF + BUFFER + PHYS_BUFFER + outer_rad;
      return (vector_t){x, center.y};
    }
  } else {
    if (center.y + outer_rad > MAX.y - WALL_BUFF - BUFFER - PHYS_BUFFER) {
      double y = MAX.y - WALL_BUFF - BUFFER - PHYS_BUFFER - outer_rad;
      return (vector_t){center.x, y};
    }
    if (center.y - outer_rad < MIN.y + WALL_BUFF + BUFFER + PHYS_BUFFER) {
      double y = MIN.y + WALL_BUFF + BUFFER + PHYS_BUFFER + outer_rad;
      return (vector_t){center.x, y};
    }
  }

  return center;
}

void compute_new_position(scene_t *scene, vector_t velocity, double dt,
                          size_t index) {
  body_t *body = scene_get_body(scene, index);
  body_set_velocity(body, velocity);
}

void compute_new_angle(scene_t *scene, double ang_velocity, double dt,
                       size_t index) {
  body_t *body = scene_get_body(scene, index);
  double rotation = body_get_rotation(body);
  if (fabs(rotation) < MAX_ROTATION) {
    body_set_angular_velocity(body, ang_velocity);
  }
}

void reset_vel(scene_t *scene, size_t index) {
  body_t *body = scene_get_body(scene, index);
  body_set_velocity(body, ZERO_VEC);
}

void reset_ang(scene_t *scene, size_t index) {
  body_t *body = scene_get_body(scene, index);
  body_set_angular_velocity(body, 0.0);
  body_reset_rotation(body);
}

void controllerHandle(char key, button_event_type_t type, double held_time,
                      int which, scene_tuple_t *scene_tup) {
  if (type == BUTTON_PRESSED) {
    switch (key) {
    case C_LB:
      if (which % 2 == PLAY1_TYPE) {
        compute_new_angle(scene_tup->scene, ANG_VELOCITY, held_time,
                          PLAY1_INDEX);
      } else {
        compute_new_angle(scene_tup->scene, ANG_VELOCITY, held_time,
                          PLAY2_INDEX);
      }
      break;
    case C_RB:
      if (which % 2 == PLAY1_TYPE) {
        compute_new_angle(scene_tup->scene, -ANG_VELOCITY, held_time,
                          PLAY1_INDEX);
      } else {
        compute_new_angle(scene_tup->scene, -ANG_VELOCITY, held_time,
                          PLAY2_INDEX);
      }
      break;
    }
  }

  if (type == BUTTON_RELEASED) {
    switch (key) {
    case C_LB:
      if (which % 2 == PLAY1_TYPE) {
        reset_ang(scene_tup->scene, PLAY1_INDEX);
      } else {
        reset_ang(scene_tup->scene, PLAY2_INDEX);
      }
      break;
    case C_RB:
      if (which % 2 == PLAY1_TYPE) {
        reset_ang(scene_tup->scene, PLAY1_INDEX);
      } else {
        reset_ang(scene_tup->scene, PLAY2_INDEX);
      }
      break;
    }
  }
}

void axisHandle(char key, axis_event_type_t type, int value, int which,
                double held_time, scene_tuple_t *scene_tup) {
  switch (key) {
  case AXIS_LEFTY:
    if (which % 2 == PLAY1_TYPE) {
      if (value < -DEAD_ZONE) {
        compute_new_position(scene_tup->scene, (vector_t){0.0, NORM_VELOCITY},
                             held_time, PLAY1_INDEX);
      } else if (value > DEAD_ZONE) {
        compute_new_position(scene_tup->scene, (vector_t){0.0, -NORM_VELOCITY},
                             held_time, PLAY1_INDEX);
      } else {
        reset_vel(scene_tup->scene, PLAY1_INDEX);
      }
    } else {
      if (value < -DEAD_ZONE) {
        compute_new_position(scene_tup->scene, (vector_t){0.0, NORM_VELOCITY},
                             held_time, PLAY2_INDEX);
      } else if (value > DEAD_ZONE) {
        compute_new_position(scene_tup->scene, (vector_t){0.0, -NORM_VELOCITY},
                             held_time, PLAY2_INDEX);
      } else {
        reset_vel(scene_tup->scene, PLAY2_INDEX);
      }
    }
    break;
  case AXIS_RIGHTX:
    if (which % 2 == PLAY1_TYPE) {
      if (value < -DEAD_ZONE) {
        compute_new_position(scene_tup->scene, (vector_t){-NORM_VELOCITY, 0.0},
                             held_time, GOAL1_INDEX);
      } else if (value > DEAD_ZONE) {
        compute_new_position(scene_tup->scene, (vector_t){NORM_VELOCITY, 0.0},
                             held_time, GOAL1_INDEX);
      } else {
        reset_vel(scene_tup->scene, GOAL1_INDEX);
      }
    } else {
      if (value < -DEAD_ZONE) {
        compute_new_position(scene_tup->scene, (vector_t){-NORM_VELOCITY, 0.0},
                             held_time, GOAL2_INDEX);
      } else if (value > DEAD_ZONE) {
        compute_new_position(scene_tup->scene, (vector_t){NORM_VELOCITY, 0.0},
                             held_time, GOAL2_INDEX);
      } else {
        reset_vel(scene_tup->scene, GOAL2_INDEX);
      }
    }

    break;
  }
}

void keyHandle(char key, key_event_type_t type, double held_time,
               scene_tuple_t *scene_tup) {
  if (type == KEY_PRESSED) {
    switch (key) {
    case A:
      compute_new_position(scene_tup->scene, (vector_t){-NORM_VELOCITY, 0.0},
                           held_time, GOAL1_INDEX);
      break;
    case W:
      compute_new_position(scene_tup->scene, (vector_t){0.0, NORM_VELOCITY},
                           held_time, PLAY1_INDEX);
      break;
    case D:
      compute_new_position(scene_tup->scene, (vector_t){NORM_VELOCITY, 0.0},
                           held_time, GOAL1_INDEX);
      break;
    case S:
      compute_new_position(scene_tup->scene, (vector_t){0.0, -NORM_VELOCITY},
                           held_time, PLAY1_INDEX);
      break;
    case Q:
      compute_new_angle(scene_tup->scene, ANG_VELOCITY, held_time, PLAY1_INDEX);
      break;
    case E:
      compute_new_angle(scene_tup->scene, -ANG_VELOCITY, held_time,
                        PLAY1_INDEX);
      break;
    case J:
      compute_new_position(scene_tup->scene, (vector_t){-NORM_VELOCITY, 0.0},
                           held_time, GOAL2_INDEX);
      break;
    case I:
      compute_new_position(scene_tup->scene, (vector_t){0.0, NORM_VELOCITY},
                           held_time, PLAY2_INDEX);
      break;
    case L:
      compute_new_position(scene_tup->scene, (vector_t){NORM_VELOCITY, 0.0},
                           held_time, GOAL2_INDEX);
      break;
    case K:
      compute_new_position(scene_tup->scene, (vector_t){0.0, -NORM_VELOCITY},
                           held_time, PLAY2_INDEX);
      break;
    case U:
      compute_new_angle(scene_tup->scene, ANG_VELOCITY, held_time, PLAY2_INDEX);
      break;
    case O:
      compute_new_angle(scene_tup->scene, -ANG_VELOCITY, held_time,
                        PLAY2_INDEX);
      break;
    case SPACE:
      if (scene_tup->game_done >= 1) {
        scene_tup->game_done = 0;
        scene_tup->end_time = 0.0;
        scene_tup->mess = "00  ||  00";
        scene_tup->score1 = 0;
        scene_tup->score2 = 0;
        snprintf(scene_tup->mess,
                 (sizeof(scene_tup->score2) + sizeof(scene_tup->score1)) + 5,
                 "%02d || %02d", scene_tup->score2, scene_tup->score1);
        reset_vel(scene_tup->scene, GOAL1_INDEX);
        reset_vel(scene_tup->scene, PLAY1_INDEX);
        reset_ang(scene_tup->scene, PLAY1_INDEX);
        reset_vel(scene_tup->scene, GOAL2_INDEX);
        reset_vel(scene_tup->scene, PLAY2_INDEX);
        reset_ang(scene_tup->scene, PLAY2_INDEX);
        body_set_centroid(scene_get_body(scene_tup->scene, BALL_INDEX),
                          (vector_t){CENTER.x, CENTER.y});
        body_set_impulse(scene_get_body(scene_tup->scene, BALL_INDEX),
                         VEC_ZERO);
        body_set_velocity(scene_get_body(scene_tup->scene, BALL_INDEX),
                          BALL_VEL);
        body_set_centroid(scene_get_body(scene_tup->scene, PLAY1_INDEX),
                          (vector_t){BUFFER, CENTER.y});
        body_set_centroid(scene_get_body(scene_tup->scene, PLAY2_INDEX),
                          (vector_t){MAX.x - BUFFER, CENTER.y});
      }
      break;
    }
  }
  if (type == KEY_RELEASED) {
    switch (key) {
    case A:
      reset_vel(scene_tup->scene, GOAL1_INDEX);
      break;
    case W:
      reset_vel(scene_tup->scene, PLAY1_INDEX);
      break;
    case D:
      reset_vel(scene_tup->scene, GOAL1_INDEX);
      break;
    case S:
      reset_vel(scene_tup->scene, PLAY1_INDEX);
      break;
    case Q:
      reset_ang(scene_tup->scene, PLAY1_INDEX);
      break;
    case E:
      reset_ang(scene_tup->scene, PLAY1_INDEX);
      break;
    case J:
      reset_vel(scene_tup->scene, GOAL2_INDEX);
      break;
    case I:
      reset_vel(scene_tup->scene, PLAY2_INDEX);
      break;
    case L:
      reset_vel(scene_tup->scene, GOAL2_INDEX);
      break;
    case K:
      reset_vel(scene_tup->scene, PLAY2_INDEX);
      break;
    case U:
      reset_ang(scene_tup->scene, PLAY2_INDEX);
      break;
    case O:
      reset_ang(scene_tup->scene, PLAY2_INDEX);
      break;
    case SPACE:
      break;
    }
  }
}

scene_t *game_init() {
  scene_t *scene = scene_init();
  scene_add_body(scene, make_player((vector_t){BUFFER, CENTER.y}, PLAYER_HEIGHT,
                                    PLAYER_WIDTH, PLAY1_TYPE));
  scene_add_body(scene, make_player((vector_t){MAX.x - BUFFER, CENTER.y},
                                    PLAYER_HEIGHT, PLAYER_WIDTH, PLAY2_TYPE));
  scene_add_body(scene, make_ball());
  make_walls(scene);
  scene_add_body(scene,
                 make_goal((vector_t){MAX.x / 4, MAX.y - (WALL_BUFF / 2)},
                           GOAL_HEIGHT, GOAL_WIDTH, GOAL1_TYPE));
  scene_add_body(scene,
                 make_goal((vector_t){MAX.x * 0.75, MIN.y + (WALL_BUFF / 2)},
                           GOAL_HEIGHT, GOAL_WIDTH, GOAL2_TYPE));
  scene_add_body(scene, make_goal((vector_t){MIN.x + (WALL_BUFF / 2), CENTER.y},
                                  STATIC_GOAL_HEIGHT, STATIC_GOAL_WIDTH,
                                  STATIC_GOAL1_TYPE));
  scene_add_body(scene, make_goal((vector_t){MAX.x - (WALL_BUFF / 2), CENTER.y},
                                  STATIC_GOAL_HEIGHT, STATIC_GOAL_WIDTH,
                                  STATIC_GOAL2_TYPE));
  return scene;
}

size_t rand_range(size_t min, size_t max) {
  return (rand() % (max - min + 1)) + min;
}

double rand_range_double(double min, double max) {
  double random = ((double)rand()) / RAND_MAX;
  double range = (max - min) * random;
  return min + range;
}

void create_pwr(scene_tuple_t *scene_tup, body_t *ball, body_t *body) {
  scene_t *scene = scene_tup->scene;
  scene_add_body(scene, body);
  create_collision(scene_tup->scene, ball, body, powerup_handler, scene_tup,
                   NULL);
}

void pwr_spawn(scene_tuple_t *scene_tup) {
  int pwr_index = rand_range(POWERUP_TYPE1, POWERUP_TYPE3);
  body_t *ball = scene_get_body(scene_tup->scene, BALL_INDEX);
  double x = rand_range_double(MIN.x + PWR_BUFFER_X, MAX.x - PWR_BUFFER_X);
  double y = rand_range_double(MIN.y + PWR_BUFFER_Y, MAX.y - PWR_BUFFER_Y);
  if (pwr_index == POWERUP_TYPE1) {
    scene_tup->pwr_type = POWERUP_TYPE1;
    body_t *body = make_powerup(x, y, POWERUP_TYPE1, SPECIAL_COLOR1);
    create_pwr(scene_tup, ball, body);
  }

  if (pwr_index == POWERUP_TYPE2) {
    scene_tup->pwr_type = POWERUP_TYPE2;
    body_t *body2 = make_powerup(x, y, POWERUP_TYPE2, SPECIAL_COLOR2);
    create_pwr(scene_tup, ball, body2);
  }

  if (pwr_index == POWERUP_TYPE3) {
    scene_tup->pwr_type = POWERUP_TYPE3;
    body_t *body3 = make_powerup(x, y, POWERUP_TYPE3, SPECIAL_COLOR3);
    create_pwr(scene_tup, ball, body3);
  }
}

void revert(scene_tuple_t *scene_tup) {
  scene_t *scene = scene_tup->scene;
  if (scene_tup->pwr_type == POWERUP_TYPE1 &&
      scene_tup->pwr_owner == PLAY1_TYPE) {
    body_t *body = scene_get_body(scene, PLAY1_INDEX);
    body_reset_rotation(body);
    body_y_scale(body, (1.0 / PLAYER_SCALAR));
  } else if (scene_tup->pwr_type == POWERUP_TYPE2 &&
             scene_tup->pwr_owner == PLAY1_TYPE) {
    body_t *body = scene_get_body(scene, STATIC_GOAL1_INDEX);
    body_reset_rotation(body);
    body_y_scale(body, (1.0 / GOAL_SCALAR));
  } else if (scene_tup->pwr_type == POWERUP_TYPE1 &&
             scene_tup->pwr_owner == PLAY2_TYPE) {
    body_t *body = scene_get_body(scene, PLAY2_INDEX);
    body_reset_rotation(body);
    body_y_scale(body, (1.0 / PLAYER_SCALAR));
  } else if (scene_tup->pwr_type == POWERUP_TYPE2 &&
             scene_tup->pwr_owner == PLAY2_TYPE) {
    body_t *body = scene_get_body(scene, STATIC_GOAL2_INDEX);
    body_reset_rotation(body);
    body_y_scale(body, (1.0 / GOAL_SCALAR));
  }
}

void spawner(scene_tuple_t *scene_tup) {
  double time = scene_tup->time;
  double update_time = scene_tup->update_time;
  double spawn_time = scene_tup->spawn_time;
  if (time >= spawn_time + update_time && scene_tup->pwr_add) {
    pwr_spawn(scene_tup);
    scene_tup->pwr_add = false;
    srand(time);
    scene_tup->spawn_time =
        rand_range_double(spawn_time, spawn_time + SPAWN_INTERVAL);
    scene_tup->update_time = scene_tup->time;
  }
}

void check_pwr_time(scene_tuple_t *scene_tup) {
  if (scene_tup->pwr_on) {
    if (scene_tup->time >= scene_tup->pwr_time + PWR_TIMER) {
      revert(scene_tup);
      scene_tup->pwr_on = false;
      scene_tup->pwr_add = true;
    }
  }
}

state_t *emscripten_init() {
  sdl_init(MIN, MAX);
  init_img("assets/pixarttp.png");
  init_text("assets/arcadeclassic.ttf", FONT_SIZE);
  init_audio("assets/paddle_hit.wav", "assets/powerup.wav",
             "assets/wall_hit.wav",
             "assets/suite_cs3_commission_main_song.wav");
  play_bg();
  state_t *state = malloc(sizeof(state_t));
  assert(state != NULL);
  state->scene = game_init();
  scene_tuple_t *sc_tup = malloc(sizeof(scene_tuple_t));
  add_forces(state);
  state->scene_tup = sc_tup;
  state->scene_tup->scene = state->scene;
  state->scene_tup->time = 0;
  state->scene_tup->update_time = 0;
  state->scene_tup->score1 = 0;
  state->scene_tup->score2 = 0;
  state->scene_tup->game_done = 0;
  state->scene_tup->mess = "00  ||  00";
  state->last_time = 0;
  state->time = 0;
  state->scene_tup->end_time = 0.0;
  state->scene_tup->pwr_on = false;
  state->scene_tup->pwr_add = true;
  state->scene_tup->spawn_time = SPAWN_INTERVAL;
  return state;
}

void emscripten_main(state_t *state) {
  double dt = time_since_last_tick();
  state->scene_tup->time += dt;
  state->time += dt;
  if (state->time >= state->last_time + 1.0) {
    srand(time(NULL));
    state->last_time = state->time;
  }

  check_pwr_time(state->scene_tup);
  spawner(state->scene_tup);
  sdl_on_key(keyHandle);
  sdl_on_axis(axisHandle);
  sdl_on_button(controllerHandle);

  body_t *play1 = scene_get_body(state->scene, PLAY1_INDEX);
  body_t *play2 = scene_get_body(state->scene, PLAY2_INDEX);

  body_t *goal1 = scene_get_body(state->scene, GOAL1_INDEX);
  body_t *goal2 = scene_get_body(state->scene, GOAL2_INDEX);
  info_t info_1 = *(info_t *)body_get_info(play1);
  info_t info_2 = *(info_t *)body_get_info(play2);
  info_t infogoal_1 = *(info_t *)body_get_info(goal1);
  info_t infogoal_2 = *(info_t *)body_get_info(goal2);

  body_set_centroid(play1, in_bounds(body_get_centroid(play1),
                                     (info_1.height / 2.0), PLAY1_TYPE));
  body_set_centroid(play2, in_bounds(body_get_centroid(play2),
                                     (info_2.height / 2.0), PLAY2_TYPE));
  body_set_centroid(goal1, in_bounds(body_get_centroid(goal1),
                                     (infogoal_1.width / 2.0), GOAL1_TYPE));
  body_set_centroid(goal2, in_bounds(body_get_centroid(goal2),
                                     (infogoal_2.width / 2.0), GOAL2_TYPE));

  scene_tick(state->scene, dt);

  if (state->scene_tup->score2 >= MAX_SCORE ||
      state->scene_tup->score1 >= MAX_SCORE) {
    if (state->scene_tup->game_done == 0) {
      state->scene_tup->game_done = 1;
    }
  }
  if (state->scene_tup->game_done >= 1) {
    printf("%f :: %d \n", state->scene_tup->end_time,
           state->scene_tup->game_done);
    if (state->scene_tup->game_done == 1) {
      state->scene_tup->end_time = state->scene_tup->time;
      state->scene_tup->game_done += 1;
    }
    if (state->scene_tup->end_time + 3.0 > state->scene_tup->time) {
      snprintf(state->scene_tup->mess,
               (sizeof(state->scene_tup->score2) +
                sizeof(state->scene_tup->score1)) +
                   MSG_SIZE,
               "%02d || %02d", state->scene_tup->score2,
               state->scene_tup->score1);
    } else {
      snprintf(state->scene_tup->mess,
               (sizeof(state->scene_tup->score2) +
                sizeof(state->scene_tup->score1)) +
                   END_SIZE,
               "THANK YOU FOR PLAYING", state->scene_tup->score2,
               state->scene_tup->score1);
    }
    sdl_clear();
    endgame_init(state->scene_tup);
    body_set_velocity(scene_get_body(state->scene, BALL_INDEX), VEC_ZERO);

  } else {
    sdl_render_scene(state->scene, state->scene_tup->mess);
  }
}

void emscripten_free(state_t *state) {
  free(state->scene_tup);
  scene_free(state->scene);
  free(state);
}