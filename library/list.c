#include <assert.h>
#include <list.h>

typedef struct list {
  void **array;
  size_t size;
  size_t alloc;
  free_func_t freer;
} list_t;

list_t *list_init(size_t initial_size, free_func_t freer) {
  list_t *list = malloc(sizeof(list_t));
  assert(list != NULL);
  list->array = malloc(sizeof(list_t *) * initial_size);
  assert(list->array != NULL);
  list->size = 0;
  list->alloc = initial_size;
  list->freer = freer;
  return list;
}

void list_free(list_t *list) {
  if (list->freer != NULL) {
    for (size_t i = 0; i < list_size(list); i++) {
      list->freer(list_get(list, i));
    }
    free(list->array);
    free(list);
  } else {
    free(list->array);
    free(list);
  }
}

size_t list_size(list_t *list) { return list->size; }

void *list_get(list_t *list, size_t index) {
  if (index > list->size || index < 0) {
    abort();
  }
  return list->array[index];
}

void list_resize(list_t *list) {
  if (list->alloc == 0) {
    list->alloc += 1;
  }
  size_t new_alloc = 2 * list->alloc;
  list->array = realloc(list->array, new_alloc * sizeof(list_t *));
  assert(list->array != NULL);
  list->alloc = new_alloc;
}

void *list_remove(list_t *list, size_t index) {
  if (list->size > 0) {
    void *ret = list->array[index];
    list->array[index] = NULL;
    for (size_t i = index + 1; i < list->size; i++) {
      list->array[i - 1] = list->array[i];
    }
    list->size -= 1;
    return ret;
  }
  abort();
}

void list_add(list_t *list, void *value) {
  if (list->size >= list->alloc) {
    list_resize(list);
  }
  if (value != NULL) {
    list->array[list->size] = value;
    list->size += 1;
  } else {
    abort();
  }
}