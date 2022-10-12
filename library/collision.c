#include "collision.h"
#include "polygon.h"
#include "vector.h"
#include <math.h>

typedef struct info {
  bool collided;
  double distance;
} overlap_info_t;

overlap_info_t check_overlap(vector_t proj1, vector_t proj2);
list_t *get_axes(list_t *shape);
vector_t projection(list_t *shape, vector_t axis);

collision_info_t find_collision(list_t *shape1, list_t *shape2) {

  list_t *axes_shape1 = get_axes(shape1);

  vector_t min_axis = (vector_t){0, 0};
  double min_dist = 100000;
  for (size_t i = 0; i < list_size(axes_shape1); i++) {
    vector_t axis = *(vector_t *)list_get(axes_shape1, i);
    vector_t proj1 = projection(shape1, axis);
    vector_t proj2 = projection(shape2, axis);
    overlap_info_t overlap_check = check_overlap(proj1, proj2);
    if (!overlap_check.collided) {
      list_free(axes_shape1);
      return (collision_info_t){false, (vector_t){0.0, 0.0}};
    } else {
      if (overlap_check.distance < min_dist) {
        min_dist = overlap_check.distance;
        min_axis = axis;
      }
    }
  }
  list_free(axes_shape1);

  list_t *axes_shape2 = get_axes(shape2);
  for (size_t i = 0; i < list_size(axes_shape2); i++) {
    vector_t axis = *(vector_t *)list_get(axes_shape2, i);
    vector_t proj1 = projection(shape1, axis);
    vector_t proj2 = projection(shape2, axis);
    overlap_info_t overlap_check = check_overlap(proj1, proj2);
    if (!overlap_check.collided) {
      list_free(axes_shape2);
      return (collision_info_t){false, (vector_t){0.0, 0.0}};
    } else {
      if (overlap_check.distance < min_dist) {
        min_dist = overlap_check.distance;
        min_axis = axis;
      }
    }
  }
  list_free(axes_shape2);
  return (collision_info_t){true, min_axis};
}

overlap_info_t check_overlap(vector_t proj1, vector_t proj2) {
  if (proj1.x < proj2.x && proj1.x < proj2.y && proj1.y < proj2.x &&
      proj1.y < proj2.y) {
    return (overlap_info_t){false, 0};
  } else if (proj2.x < proj1.x && proj2.x < proj1.y && proj2.y < proj1.x &&
             proj2.y < proj1.y) {
    return (overlap_info_t){false, 0};
  }
  double min_val = fabs(proj2.y - proj1.x);
  if (proj2.x > proj1.x) {
    min_val = fabs(proj1.y - proj2.x);
  }
  return (overlap_info_t){true, min_val};
}

list_t *get_axes(list_t *shape) {
  list_t *axes = list_init(list_size(shape), (free_func_t)free);
  for (size_t i = 0; i < list_size(shape); i++) {
    vector_t vertex1 = *(vector_t *)list_get(shape, i);
    vector_t vertex2 = *(vector_t *)list_get(shape, (i + 1) % list_size(shape));

    vector_t edge = vec_subtract(vertex1, vertex2);
    double edge_magnitude = sqrt(pow(edge.x, 2) + pow(edge.y, 2));
    edge = vec_multiply(1 / edge_magnitude, edge);
    vector_t *normal = malloc(sizeof(vector_t));
    normal->x = -1.0 * edge.y;
    normal->y = edge.x;
    list_add(axes, normal);
  }
  return axes;
}

vector_t projection(list_t *shape, vector_t axis) {
  vector_t dummy = *(vector_t *)list_get(shape, 0);
  vector_t min_max_proj =
      (vector_t){vec_dot(axis, dummy), vec_dot(axis, dummy)};
  for (size_t i = 0; i < list_size(shape); i++) {
    double proj = vec_dot(axis, *(vector_t *)list_get(shape, i));
    if (proj > min_max_proj.y) {
      min_max_proj.y = proj;
    } else if (proj < min_max_proj.x) {
      min_max_proj.x = proj;
    }
  }
  return min_max_proj;
}