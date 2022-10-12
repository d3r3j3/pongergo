#include "scene.h"
#include "forces.h"
#include <assert.h>
#include <stdlib.h>

const size_t init_body_num = 100;

typedef struct force {
  force_creator_t force;
  void *aux;
  free_func_t freer;
  list_t *bodies;
} force_t;

typedef struct scene {
  list_t *body_array;
  list_t *forces;
  size_t size;
  size_t capacity;
} scene_t;

scene_t *scene_init(void) {
  scene_t *scene = malloc(sizeof(scene_t));
  assert(scene != NULL);

  scene->body_array = list_init(init_body_num, (free_func_t)body_free);
  assert(scene->body_array != NULL);

  scene->forces = list_init(2, NULL);
  assert(scene->forces != NULL);

  scene->size = 0;
  scene->capacity = init_body_num;
  return scene;
}

void scene_forces_free(scene_t *scene) {
  for (size_t i = 0; i < list_size(scene->forces); i++) {
    force_t *force = list_get(scene->forces, i);
    free_func_t aux_free = force->freer;
    if (aux_free != NULL) {
      aux_free(force->aux);
    }
    list_free(force->bodies);
    free(force);
  }
}

void scene_free(scene_t *scene) {
  scene_forces_free(scene);
  list_free(scene->forces);
  list_free(scene->body_array);
  free(scene);
}

size_t scene_bodies(scene_t *scene) { return list_size(scene->body_array); }

body_t *scene_get_body(scene_t *scene, size_t index) {
  return list_get(scene->body_array, index);
}

void scene_add_body(scene_t *scene, body_t *body) {
  list_add(scene->body_array, body);
}

void scene_remove_body(scene_t *scene, size_t index) {
  body_remove(list_get(scene->body_array, index));
}

void scene_add_force_creator(scene_t *scene, force_creator_t forcer, void *aux,
                             free_func_t freer) {
  list_t *bodies = list_init(2, NULL);
  scene_add_bodies_force_creator(scene, forcer, aux, bodies, freer);
}

void scene_add_bodies_force_creator(scene_t *scene, force_creator_t forcer,
                                    void *aux, list_t *bodies,
                                    free_func_t freer) {
  force_t *force = malloc(sizeof(force_t));
  force->bodies = bodies;
  force->aux = aux;
  force->force = forcer;
  force->freer = freer;
  list_add(scene->forces, force);
}

void apply_forces(scene_t *scene, double dt) {
  for (size_t i = 0; i < list_size(scene->forces); i++) {
    force_t *f = list_get(scene->forces, i);
    force_creator_t forcer = f->force;
    forcer(f->aux);
  }

  for (size_t i = 0; i < scene_bodies(scene); i++) {
    body_tick(scene_get_body(scene, i), dt);
  }
}

void remove_forces(scene_t *scene) {
  size_t count = 0;
  while (count < list_size(scene->forces)) {
    force_t *f = list_get(scene->forces, count);
    list_t *body_col = f->bodies;
    bool remove = false;
    for (size_t i = 0; i < list_size(body_col); i++) {
      remove = body_is_removed(list_get(body_col, i));
      if (remove) {
        free_func_t aux_free = f->freer;
        if (aux_free != NULL) {
          aux_free(f->aux);
        }
        list_free(f->bodies);
        free(f);
        list_remove(scene->forces, count);
        break;
      }
    }
    if (!remove) {
      count++;
    }
  }

  size_t cnt = 0;
  while (cnt < scene_bodies(scene)) {
    body_t *body = scene_get_body(scene, cnt);
    if (body_is_removed(body)) {
      body_free(body);
      list_remove(scene->body_array, cnt);
    } else {
      cnt++;
    }
  }
}

void scene_tick(scene_t *scene, double dt) {
  apply_forces(scene, dt);
  remove_forces(scene);
}