#include "star.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const double THETA = M_PI / 2;

typedef struct star {
  vec_list_t *star_list;
  int num_points;

  // size
  double in_rad;
  double out_rad;
  vector_t centroid;
  double weight;

  // color
  rgb_color_t color;

  // movement
  vector_t *velocity;
  double ang_vel;
  double elasticity;

} star_t;

star_t *star_t_init(size_t star_points) {
  star_t *new_star = malloc(sizeof(star_t));
  assert(new_star != NULL);

  new_star->num_points = star_points;
  new_star->centroid = (vector_t){100, 400};
  new_star->velocity = malloc(sizeof(vector_t));
  new_star->velocity->x = 30;
  new_star->velocity->y = 0;
  new_star->ang_vel = -2.0;
  return new_star;
}

void make_star(star_t *star, double i_rad, double o_rad, rgb_color_t color,
               double weight, double elasticity) {
  double theta = THETA;
  double d_theta = M_PI * (2.0 / star->num_points);
  double inner_theta = THETA - 0.5 * d_theta;

  star->star_list = vec_list_init(star->num_points * 2);
  assert(star->star_list != NULL);
  for (size_t i = 0; i < star->num_points; i++) {
    vector_t *star_point = malloc(sizeof(vector_t));
    assert(star_point != NULL);
    vector_t *inner_point = malloc(sizeof(vector_t));
    assert(inner_point != NULL);
    star_point->x = star->centroid.x + o_rad * cos(theta);
    star_point->y = star->centroid.y + o_rad * sin(theta);
    inner_point->x = star->centroid.x + i_rad * cos(inner_theta);
    inner_point->y = star->centroid.y + i_rad * sin(inner_theta);
    vec_list_add(star->star_list, inner_point);
    vec_list_add(star->star_list, star_point);
    inner_theta += d_theta;
    theta += d_theta;
  }

  star->color = color;

  star->out_rad = o_rad;
  star->in_rad = i_rad;
  star->centroid = polygon_centroid((list_t *)star->star_list);
  star->weight = weight;
  star->elasticity = elasticity;
}

void star_rot(star_t *star, double angle) {
  polygon_rotate((list_t *)star->star_list, angle, star->centroid);
  star->ang_vel = angle;
}

vec_list_t *star_get_vertices(star_t *star) { return star->star_list; }

vector_t *star_get_velocity(star_t *star) { return star->velocity; }

void star_free(star_t *star) {
  vec_list_free(star->star_list);
  free(star->velocity);
  free(star);
}

vector_t star_get_centroid(star_t *star) { return star->centroid; }

double star_get_in_rad(star_t *star) { return star->in_rad; }

double star_get_out_rad(star_t *star) { return star->out_rad; }

double star_get_weight(star_t *star) { return star->weight; }

double star_get_ang_vel(star_t *star) { return star->ang_vel; }

double star_get_elasticity(star_t *star) { return star->elasticity; }

rgb_color_t star_get_color(star_t *star) { return star->color; }
