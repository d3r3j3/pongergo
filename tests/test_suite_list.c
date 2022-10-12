#include "list.h"
#include "test_util.h"
#include "vec_list.h"
#include <assert.h>
#include <stdlib.h>

vec_list_t *create_vec_list(size_t n, double x, double y) {
  vec_list_t *vl = vec_list_init(n);
  for (size_t i = 0; i < n; i++) {
    vector_t *v = malloc(sizeof(*v));
    v->x = x;
    v->y = y;
    vec_list_add(vl, v);
  }
  return vl;
}

void test_list_size0() {
  list_t *pl = list_init(0, (free_func_t)vec_list_free);
  assert(list_size(pl) == 0);
  list_free(pl);
}

void test_list_size1() {
  list_t *pl = list_init(1, (free_func_t)vec_list_free);
  assert(list_size(pl) == 0);
  // Add
  vec_list_t *vl = create_vec_list(1, 0, 0);
  list_add(pl, vl);
  assert(list_size(pl) == 1);
  // Remove
  assert(list_remove(pl, 0) == vl);
  vec_list_free(vl);
  assert(list_size(pl) == 0);
  // Add again
  vl = create_vec_list(1, 1, 1);
  list_add(pl, vl);
  assert(list_size(pl) == 1);
  // Modify
  vec_list_get(list_get(pl, 0), 0)->x = 1;
  vec_list_get(list_get(pl, 0), 0)->y = 2;
  vector_t va = *vec_list_get(list_get(pl, 0), 0);
  assert(vec_equal(va, (vector_t){1, 2}));
  list_free(pl);
}
#define LARGE_SIZE 10000

void test_list_large_get_set() {
  list_t *pl = list_init(LARGE_SIZE, (free_func_t)vec_list_free);
  // Add to capacity
  for (size_t i = 0; i < LARGE_SIZE; i++) {
    vec_list_t *vl = create_vec_list(i, 0, 0);
    list_add(pl, vl);
  }

  // Check
  for (size_t i = 0; i < LARGE_SIZE; i++) {
    size_t curr_size = vec_list_size(list_get(pl, i));
    assert(curr_size == i);
    vec_list_t *vl = list_get(pl, i);
    for (size_t j = 0; j < curr_size; j++) {
      assert(vec_equal(*vec_list_get(vl, j), (vector_t){0, 0}));
    }
  }

  // Set every 100th value
  for (size_t i = 0; i < LARGE_SIZE; i++) {
    size_t curr_size = vec_list_size(list_get(pl, i));
    vec_list_t *vl = list_get(pl, i);
    for (size_t j = 0; j < curr_size; j += 100) {
      vector_t *v = vec_list_get(vl, j);
      v->x = v->y = j * 10;
    }
  }

  // Check all values again
  for (size_t i = 0; i < LARGE_SIZE; i++) {
    size_t curr_size = vec_list_size(list_get(pl, i));
    assert(curr_size == i);
    vec_list_t *vl = list_get(pl, i);
    for (size_t j = 0; j < curr_size; j += 100) {
      double value = j * 10;
      assert(vec_equal(*vec_list_get(vl, j), (vector_t){value, value}));
    }
  }

  /* for (size_t i = 0; i < LARGE_SIZE; i++) {
    vec_list_free(list_get(pl, i));
  } */

  list_free(pl);
}

int main(int argc, char *argv[]) {
  // Run all tests if there are no command-line arguments
  bool all_tests = argc == 1;
  // Read test name from file
  char testname[100];
  if (!all_tests) {
    read_testname(argv[1], testname, sizeof(testname));
  }

  DO_TEST(test_list_size0)
  DO_TEST(test_list_size1)
  DO_TEST(test_list_large_get_set)

  puts("list_test PASS");
}