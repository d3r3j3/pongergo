#include "vector.h"
#include <math.h>
#include <stdlib.h>

const vector_t VEC_ZERO = {.x = 0, .y = 0};

vector_t vec_add(vector_t v1, vector_t v2) {
  vector_t sum_v = {.x = v1.x + v2.x, .y = v1.y + v2.y};

  return sum_v;
}

vector_t vec_subtract(vector_t v1, vector_t v2) {
  vector_t difference_v = vec_add(v1, vec_negate(v2));

  return difference_v;
}

vector_t vec_negate(vector_t v) {
  vector_t negate_v = vec_multiply(-1.0, v);

  return negate_v;
}

vector_t vec_multiply(double scalar, vector_t v) {
  vector_t product_v = {.x = v.x * scalar, .y = v.y * scalar};

  return product_v;
}

double vec_dot(vector_t v1, vector_t v2) { return v1.x * v2.x + v1.y * v2.y; }

double vec_cross(vector_t v1, vector_t v2) { return v1.x * v2.y - v1.y * v2.x; }

vector_t vec_rotate(vector_t v, double angle) {
  vector_t rotated_v = {.x = cos(angle) * v.x - sin(angle) * v.y,
                        .y = sin(angle) * v.x + cos(angle) * v.y};

  return rotated_v;
}
