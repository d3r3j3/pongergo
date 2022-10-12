#include "star.h"
#include "test_util.h"
#include "vec_list.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

const double INIT_THETA = (M_PI / 2);
const double OUTER_RADIUS = 50;
const double INNER_RADIUS = 25;
const rgb_color_t color = (rgb_color_t){0.1, 0.1, 0.1};
const vector_t CENTER = (vector_t){100, 400};

vec_list_t *star(size_t n, size_t r, size_t inner_r, vector_t center_vec) {
  double theta = INIT_THETA;
  double d_theta = M_PI * (2.0 / n);
  double inner_theta = INIT_THETA - 0.5 * d_theta;

  vec_list_t *star_vec = vec_list_init(n * 2);
  for (size_t i = 0; i < n; i++) {
    vector_t *star_point = malloc(sizeof(vector_t));
    assert(star_point != NULL);
    vector_t *inner_point = malloc(sizeof(vector_t));
    assert(inner_point != NULL);
    star_point->x = center_vec.x + r * cos(theta);
    star_point->y = center_vec.y + r * sin(theta);
    inner_point->x = center_vec.x + inner_r * cos(inner_theta);
    inner_point->y = center_vec.y + inner_r * sin(inner_theta);

    vec_list_add(star_vec, inner_point);
    vec_list_add(star_vec, star_point);

    inner_theta += d_theta;
    theta += d_theta;
  }
  return star_vec;
}

#define MAX_POINTS 100

void test_star_max_points() {

  for (size_t i = 0; i < MAX_POINTS; i++) {
    vec_list_t *star_vec = star(i, OUTER_RADIUS, INNER_RADIUS, CENTER);
    star_t *my_star = star_t_init(i);
    make_star(my_star, INNER_RADIUS, OUTER_RADIUS, color, 0.1, 0.1);
    for (size_t j = 0; j < i * 2; j++) {
      assert(vec_equal(*vec_list_get(star_get_vertices(my_star), j),
                       *vec_list_get(star_vec, j)));
    }
    star_free(my_star);
    vec_list_free(star_vec);
  }
}

int main(int argc, char *argv[]) {
  // Run all tests if there are no command-line arguments
  bool all_tests = argc == 1;
  // Read test name from file
  char testname[100];
  if (!all_tests) {
    read_testname(argv[1], testname, sizeof(testname));
  }

  DO_TEST(test_star_max_points)

  puts("list_test PASS");
}