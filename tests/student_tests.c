#include "body.h"
#include "forces.h"
#include "test_util.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

list_t *make_shape() {
  list_t *shape = list_init(4, free);
  vector_t *v = malloc(sizeof(*v));
  *v = (vector_t){-1, -1};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){+1, -1};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){+1, +1};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){-1, +1};
  list_add(shape, v);
  return shape;
}

// Tests that G changes force of gravity proportionally.
// Tests that scene tick resets forces to 0 every tick.
void test_gravity() {
  const double M1 = 3.0, M2 = 10.3;
  const double G = 1e3;
  const double G2 = 3e3;
  const double DT = 1e-6;
  const int STEPS = 1000000;

  scene_t *scene = scene_init();

  // same mass as mass3, smaller G
  body_t *mass1 = body_init(make_shape(), M1, (rgb_color_t){0, 0, 0});
  body_set_velocity(mass1, (vector_t){10, 20});
  scene_add_body(scene, mass1);

  // larger mass
  body_t *mass2 = body_init(make_shape(), M2, (rgb_color_t){0, 0, 0});
  body_set_centroid(mass2, (vector_t){100, 200});
  scene_add_body(scene, mass2);

  // same mass as mass1, velocity, higher G
  body_t *mass3 = body_init(make_shape(), M1, (rgb_color_t){0, 0, 0});
  body_set_velocity(mass3, (vector_t){10, 20});
  scene_add_body(scene, mass3);

  create_newtonian_gravity(scene, G, mass1, mass2);
  create_newtonian_gravity(scene, G2, mass3, mass2);

  for (int i = 0; i < STEPS; i++) {
    if (i > 1) {
      assert(fabs(body_get_velocity(mass3).y) >
             fabs(body_get_velocity(mass1).y));
    }
    scene_tick(scene, DT);
  }
  scene_free(scene);
}

// Tests that K value impacts the spring force proportionally
// and that the force resets to 0 with each tick.
void test_spring() {
  const double M = 10;
  const double K = 2;
  const double K2 = 10;
  const double A = 3;
  const double DT = 1e-6;
  const int STEPS = 1000000;
  scene_t *scene = scene_init();

  body_t *mass1 = body_init(make_shape(), M, (rgb_color_t){0, 0, 0});
  body_set_centroid(mass1, (vector_t){A, 0});
  scene_add_body(scene, mass1);

  body_t *anchor = body_init(make_shape(), INFINITY, (rgb_color_t){0, 0, 0});
  body_set_centroid(anchor, (vector_t){6, 0});
  scene_add_body(scene, anchor);

  // Same mass as mass1, larger K value
  body_t *mass2 = body_init(make_shape(), M, (rgb_color_t){0, 0, 0});
  body_set_centroid(mass2, (vector_t){9, 0});
  scene_add_body(scene, mass2);

  create_spring(scene, K, mass1, anchor);
  create_spring(scene, K2, mass2, anchor);

  for (int i = 0; i < STEPS; i++) {
    if (i > 1) {
      assert(fabs(body_get_velocity(mass2).x) >
             fabs(body_get_velocity(mass1).x));
    }
    scene_tick(scene, DT);
  }
  scene_free(scene);
}

// Tests that drag has a proportional effect on velocity and that smaller drag
// constant equates to lower velocity.
void test_drag() {
  const double M = 10;
  const double gamma = 0.1;
  const double gamma2 = 0.0001;
  const double DT = 1e-6;
  const int STEPS = 1000000;
  scene_t *scene = scene_init();

  // no drag applied
  body_t *mass1 = body_init(make_shape(), M, (rgb_color_t){0, 0, 0});
  body_set_velocity(mass1, (vector_t){0, 20});
  scene_add_body(scene, mass1);

  // larger drag constant applied
  body_t *mass2 = body_init(make_shape(), M, (rgb_color_t){0, 0, 0});
  body_set_velocity(mass2, (vector_t){0, 20});
  scene_add_body(scene, mass2);

  // smaller drag constant applied
  body_t *mass3 = body_init(make_shape(), M, (rgb_color_t){0, 0, 0});
  body_set_velocity(mass3, (vector_t){0, 20});
  scene_add_body(scene, mass3);

  create_drag(scene, gamma, mass2);
  create_drag(scene, gamma2, mass3);

  for (int i = 0; i < STEPS; i++) {
    if (i > 1) {
      assert(fabs(body_get_velocity(mass2).y) <
             fabs(body_get_velocity(mass1).y));
    }
    scene_tick(scene, DT);
  }
  scene_free(scene);
}

int main(int argc, char *argv[]) {
  // Run all tests if there are no command-line arguments
  bool all_tests = argc == 1;
  // Read test name from file
  char testname[100];
  if (!all_tests) {
    read_testname(argv[1], testname, sizeof(testname));
  }

  DO_TEST(test_gravity)
  DO_TEST(test_spring)
  DO_TEST(test_drag)

  puts("student_tests PASS");
}
