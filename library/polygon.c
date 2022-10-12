#include "polygon.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

double polygon_area(list_t *polygon) {
  double double_area = 0;
  size_t size = vec_list_size((vec_list_t *)polygon);
  if (size < 2) {
    return double_area;
  }

  for (size_t i = 0; i < size; i++) {
    double_area += vec_cross(*(vector_t *)list_get(polygon, i),
                             *(vector_t *)list_get(polygon, (i + 1) % size));
  }

  if (double_area < 0) {
    double_area *= -1;
  }
  return double_area / 2;
}

vector_t polygon_centroid(list_t *polygon) {
  size_t size = vec_list_size((vec_list_t *)polygon);
  double centroid_x = 0;
  double centroid_y = 0;
  for (size_t i = 0; i < size; i++) {
    vector_t current_vec = *(vector_t *)list_get(polygon, i);
    vector_t next_vec = *(vector_t *)list_get(polygon, (i + 1) % size);
    centroid_x +=
        (current_vec.x + next_vec.x) * vec_cross(current_vec, next_vec);
    centroid_y +=
        (current_vec.y + next_vec.y) * vec_cross(current_vec, next_vec);
  }

  double area = polygon_area(polygon);
  vector_t *centroid_vec = malloc(sizeof(vector_t));
  assert(centroid_vec != NULL);
  centroid_vec->x = (1 / (6 * area)) * centroid_x;
  centroid_vec->y = (1 / (6 * area)) * centroid_y;
  vector_t final_centroid = *centroid_vec;
  free(centroid_vec);
  return final_centroid;
}

void polygon_translate(list_t *polygon, vector_t translation) {
  for (size_t i = 0; i < vec_list_size((vec_list_t *)polygon); i++) {
    vector_t *current_vec = vec_list_get((vec_list_t *)polygon, i);
    vector_t sum = vec_add(*current_vec, translation);
    current_vec->x = sum.x;
    current_vec->y = sum.y;
  }
}

void polygon_rotate(list_t *polygon, double angle, vector_t point) {
  for (size_t i = 0; i < vec_list_size((vec_list_t *)polygon); i++) {
    vector_t *current_vec = vec_list_get((vec_list_t *)polygon, i);
    vector_t to_origin_vec = vec_subtract(*current_vec, point);
    vector_t rotate_vec = vec_rotate(to_origin_vec, angle);
    vector_t final_vec = vec_add(rotate_vec, point);
    current_vec->x = final_vec.x;
    current_vec->y = final_vec.y;
  }
}