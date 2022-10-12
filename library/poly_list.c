#include "list.h"
#include "star.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct poly_list {
  void **array;
  size_t size;
  size_t alloc;
} poly_list_t;

poly_list_t *poly_list_init(size_t initial_size) {
  return (poly_list_t *)list_init(initial_size, (free_func_t)list_free);
}

void poly_list_free(poly_list_t *list) { list_free((list_t *)list); }

void poly_list_array_free(poly_list_t *list) {
  free(list->array);
  free(list);
}

size_t poly_list_size(poly_list_t *list) { return list_size((list_t *)list); }

void *poly_list_get(poly_list_t *list, size_t index) {
  return list_get((list_t *)list, index);
}

void poly_list_add(poly_list_t *list, void *value) {
  list_add((list_t *)list, value);
}

void *poly_list_remove(poly_list_t *list) {
  return list_remove((list_t *)list, list_size((list_t *)list) - 1);
}

void *poly_list_remove_index(poly_list_t *list, size_t index) {
  return list_remove((list_t *)list, index);
}
