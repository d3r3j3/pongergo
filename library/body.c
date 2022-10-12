#include "polygon.h"
#include <assert.h>
#include <body.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef struct body {
  list_t *shape;
  double rotation;
  double max_rotation;
  vector_t velocity;
  double max_velocity;
  double ang_velocity;
  vector_t force;
  vector_t impulse;
  double mass;
  rgb_color_t color;
  bool removed;
  void *info;
  free_func_t info_freer;
} body_t;

body_t *body_init(list_t *shape, double mass, rgb_color_t color) {
  body_t *body = malloc(sizeof(body_t));
  assert(body != NULL);
  body->rotation = 0.0;
  body->max_rotation = 360.0;
  body->force = (vector_t){.x = 0.0, .y = 0.0};
  body->impulse = (vector_t){.x = 0.0, .y = 0.0};
  body->velocity = (vector_t){.x = 0.0, .y = 0.0};
  body->shape = shape;
  body->color = color;
  body->mass = mass;
  body->removed = false;
  body->info = NULL;
  body->info_freer = NULL;
  return body;
}

body_t *body_init_with_info(list_t *shape, double mass, rgb_color_t color,
                            void *info, free_func_t info_freer) {
  body_t *body = malloc(sizeof(body_t));
  assert(body != NULL);
  body->force = (vector_t){.x = 0.0, .y = 0.0};
  body->impulse = (vector_t){.x = 0.0, .y = 0.0};
  body->velocity = (vector_t){.x = 0.0, .y = 0.0};
  body->max_velocity = __DBL_MAX__;
  body->ang_velocity = 0.0;
  body->shape = shape;
  body->color = color;
  body->removed = false;
  body->mass = mass;
  body->info = info;
  body->info_freer = info_freer;
  return body;
}

void body_free(body_t *body) {
  if (body->info_freer != NULL) {
    body->info_freer(body->info);
  }
  list_free(body->shape);
  free(body);
}

list_t *body_get_shape(body_t *body) {
  list_t *poly = list_init(list_size(body->shape), (free_func_t)free);
  for (size_t i = 0; i < list_size(body->shape); i++) {
    vector_t *vec = list_get(body->shape, i);
    vector_t *new_vec = malloc(sizeof(vector_t));
    new_vec->x = vec->x;
    new_vec->y = vec->y;
    list_add(poly, new_vec);
  }
  return poly;
}

vector_t body_get_centroid(body_t *body) {
  return polygon_centroid(body->shape);
}

vector_t body_get_velocity(body_t *body) { return body->velocity; }

double body_get_angular_velocity(body_t *body) { return body->ang_velocity; }

double body_get_rotation(body_t *body) { return body->rotation; }

double body_get_mass(body_t *body) { return body->mass; }

rgb_color_t body_get_color(body_t *body) { return body->color; }

void *body_get_info(body_t *body) { return body->info; }

void body_y_scale(body_t *body, double scalar) {
  list_t *vectors = body->shape;
  vector_t centroid = body_get_centroid(body);
  for (size_t i = 0; i < list_size(vectors); i++) {
    vector_t *vec = list_get(vectors, i);
    vec->y = (scalar * (vec->y - centroid.y)) + centroid.y;
  }
}

void body_set_centroid(body_t *body, vector_t x) {
  polygon_translate(body->shape,
                    vec_subtract(x, polygon_centroid(body->shape)));
}

void body_set_color(body_t *body, rgb_color_t color) { body->color = color; }

void body_set_velocity(body_t *body, vector_t v) { body->velocity = v; }

void body_set_max_velocity(body_t *body, double m) { body->max_velocity = m; }

void body_set_angular_velocity(body_t *body, double v) {
  body->ang_velocity = v;
}

void body_set_rotation(body_t *body, double angle) {
  if (fabs(body->rotation + angle) <= body->max_rotation) {
    polygon_rotate(body->shape, angle, body_get_centroid(body));
    body->rotation = body->rotation + angle;
  } else {
    body->ang_velocity = 0.0;
  }
  if (fabs(body->rotation) > 2 * M_PI) {
    body->rotation -= 2 * M_PI;
  }
}

void body_set_max_rotation(body_t *body, double angle) {
  body->max_rotation = angle;
}

void body_reset_rotation(body_t *body) {
  body_set_rotation(body, -(body->rotation));
}

void body_add_force(body_t *body, vector_t force) {
  body->force = vec_add(body->force, force);
}

void body_set_force(body_t *body, vector_t force) { body->force = force; }

vector_t body_get_force(body_t *body) { return body->force; }

void body_add_impulse(body_t *body, vector_t impulse) {
  body->impulse = vec_add(body->impulse, impulse);
}

void body_set_impulse(body_t *body, vector_t impulse) {
  body->impulse = impulse;
}

void body_tick(body_t *body, double dt) {
  vector_t dv_f = vec_multiply(dt / body->mass, body->force);
  vector_t dv_i = vec_multiply(1.0 / body->mass, body->impulse);
  vector_t final_velocity = vec_add(body->velocity, vec_add(dv_f, dv_i));
  if (fabs(final_velocity.x) > body->max_velocity) {
    final_velocity.x =
        body->max_velocity * (final_velocity.x / fabs(final_velocity.x));
  }
  if (fabs(final_velocity.y) > body->max_velocity) {
    final_velocity.y =
        body->max_velocity * (final_velocity.y / fabs(final_velocity.y));
  }
  vector_t avg_velocity =
      vec_multiply(0.5, vec_add(body->velocity, final_velocity));
  vector_t displacement = vec_multiply(dt, avg_velocity);
  body_set_centroid(body, vec_add(body_get_centroid(body), displacement));
  body_set_rotation(body, (body->ang_velocity) * dt);
  body_set_velocity(body, final_velocity);
  body_set_force(body, (vector_t){0, 0});
  body_set_impulse(body, (vector_t){0, 0});
}

void body_remove(body_t *body) {
  if (!body_is_removed(body)) {
    body->removed = true;
  }
}

bool body_is_removed(body_t *body) { return body->removed; }