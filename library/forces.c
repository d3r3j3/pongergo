#include "collision.h"
#include <forces.h>
#include <math.h>

typedef struct info {
  int body_type;
  double width;
  double height;
  int special_type;
} info_t;

void free_aux(aux_t *aux) { free(aux); }

void newtonian_handler(void *aux);
void spring_handler(void *aux);
void drag_handler(void *aux);
void collision_handler(void *aux);
void force_collision_handler(void *aux);
void physics_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                               void *aux);
void angular_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                               void *aux);

void create_newtonian_gravity(scene_t *scene, double G, body_t *body1,
                              body_t *body2) {
  aux_t *aux = malloc(sizeof(aux_t));
  aux->force_const = G;
  aux->bodies = list_init(2, NULL);
  list_add(aux->bodies, body1);
  list_add(aux->bodies, body2);
  scene_add_bodies_force_creator(scene, newtonian_handler, aux, aux->bodies,
                                 (free_func_t)free_aux);
}

void newtonian_handler(void *aux) {
  aux_t *aux_h = aux;
  vector_t center_1 = body_get_centroid(list_get(aux_h->bodies, 0));
  vector_t center_2 = body_get_centroid(list_get(aux_h->bodies, 1));
  double distance =
      sqrt(pow(center_2.x - center_1.x, 2) + pow(center_2.y - center_1.y, 2));
  if (distance > 100) {
    vector_t unit_vec =
        vec_multiply(1.0 / distance, vec_subtract(center_2, center_1));
    double m1 = body_get_mass(list_get(aux_h->bodies, 0));
    double m2 = body_get_mass(list_get(aux_h->bodies, 1));
    double scalar =
        (-1.0 * aux_h->force_const * m1 * m2) * (1 / pow(distance, 2));
    vector_t force = vec_multiply(scalar, unit_vec);
    body_add_force(list_get(aux_h->bodies, 1), force);
    body_add_force(list_get(aux_h->bodies, 0), vec_negate(force));
  }
}

void create_spring(scene_t *scene, double k, body_t *body1, body_t *body2) {
  aux_t *aux = malloc(sizeof(aux_t));
  aux->force_const = k;
  aux->bodies = list_init(2, NULL);
  list_add(aux->bodies, body1);
  list_add(aux->bodies, body2);
  scene_add_bodies_force_creator(scene, spring_handler, aux, aux->bodies,
                                 (free_func_t)free_aux);
}

void spring_handler(void *aux) {
  aux_t *aux_h = aux;
  vector_t center_1 = body_get_centroid(list_get(aux_h->bodies, 0));
  vector_t center_2 = body_get_centroid(list_get(aux_h->bodies, 1));
  vector_t diff_vec = vec_subtract(center_2, center_1);
  vector_t spr_force = vec_multiply(aux_h->force_const, diff_vec);
  body_add_force(list_get(aux_h->bodies, 0), spr_force);
  body_add_force(list_get(aux_h->bodies, 1), vec_negate(spr_force));
}

void create_drag(scene_t *scene, double gamma, body_t *body) {
  aux_t *aux = malloc(sizeof(aux_t));
  aux->force_const = gamma;
  aux->bodies = list_init(1, NULL);
  list_add(aux->bodies, body);
  scene_add_bodies_force_creator(scene, drag_handler, aux, aux->bodies,
                                 (free_func_t)free_aux);
}

void drag_handler(void *aux) {
  aux_t *aux_h = aux;
  vector_t velocity = body_get_velocity(list_get(aux_h->bodies, 0));
  vector_t force = vec_multiply(-1.0 * aux_h->force_const, velocity);
  body_add_force(list_get(aux_h->bodies, 0), force);
}

void create_collision(scene_t *scene, body_t *body1, body_t *body2,
                      collision_handler_t handler, void *aux,
                      free_func_t freer) {

  if (freer != NULL) {
    aux_t *aux_f = aux;
    aux_f->handle = handler;
    aux_f->prev_tick = false;
    aux_f->scene = NULL;
    scene_add_bodies_force_creator(scene, force_collision_handler, aux_f,
                                   aux_f->bodies, freer);
  } else {
    aux_t *aux_n = malloc(sizeof(aux_t));
    aux_n->bodies = list_init(2, NULL);
    list_add(aux_n->bodies, body1);
    list_add(aux_n->bodies, body2);
    aux_n->handle = handler;
    aux_n->prev_tick = false;
    aux_n->scene = aux;
    scene_add_bodies_force_creator(scene, force_collision_handler, aux_n,
                                   aux_n->bodies, (free_func_t)free_aux);
  }
}

void force_collision_handler(void *aux) {
  aux_t *aux_f = aux;
  body_t *body1 = list_get(aux_f->bodies, 0);
  body_t *body2 = list_get(aux_f->bodies, 1);
  list_t *shape1 = body_get_shape(body1);
  list_t *shape2 = body_get_shape(body2);
  void *aux_c = aux_f->scene;
  if (aux_c == NULL) {
    aux_c = aux_f;
  }

  collision_info_t info = find_collision(shape1, shape2);
  if (!aux_f->prev_tick && info.collided) {
    collision_handler_t handle = aux_f->handle;
    handle(body1, body2, info.axis, aux_c);
    aux_f->prev_tick = true;
  } else if (aux_f->prev_tick && !info.collided) {
    aux_f->prev_tick = false;
  }
  list_free(shape1);
  list_free(shape2);
}

void create_destructive_collision(scene_t *scene, body_t *body1,
                                  body_t *body2) {
  aux_t *aux = malloc(sizeof(aux_t));
  aux->bodies = list_init(2, NULL);
  list_add(aux->bodies, body1);
  list_add(aux->bodies, body2);
  scene_add_bodies_force_creator(scene, collision_handler, aux, aux->bodies,
                                 (free_func_t)free_aux);
}

void create_destructive_one_body_collision(scene_t *scene, body_t *body1,
                                           body_t *body2) {
  aux_t *aux = malloc(sizeof(aux_t));
  aux->bodies = list_init(2, NULL);
  list_add(aux->bodies, body1);
  list_add(aux->bodies, body2);
  scene_add_bodies_force_creator(scene, collision_handler, aux, aux->bodies,
                                 (free_func_t)free_aux);
}

void collision_handler(void *aux) {
  aux_t *aux_c = aux;
  body_t *body1 = list_get(aux_c->bodies, 0);
  body_t *body2 = list_get(aux_c->bodies, 1);
  list_t *shape1 = body_get_shape(body1);
  list_t *shape2 = body_get_shape(body2);
  if (find_collision(shape1, shape2).collided) {
    body_remove(body1);
    body_remove(body2);
  }
  list_free(shape1);
  list_free(shape2);
}

void create_physics_collision(scene_t *scene, double elasticity, body_t *body1,
                              body_t *body2) {
  aux_t *aux = malloc(sizeof(aux_t));
  aux->bodies = list_init(2, NULL);
  aux->force_const = elasticity;
  list_add(aux->bodies, body1);
  list_add(aux->bodies, body2);
  create_collision(scene, body1, body2, physics_collision_handler, aux,
                   (free_func_t)free_aux);
}

void physics_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                               void *aux) {
  aux_t *aux_h = aux;
  vector_t collision_axis = axis;
  double mass1 = body_get_mass(body1);
  double mass2 = body_get_mass(body2);
  double u1 = vec_dot(body_get_velocity(body1), collision_axis);
  double u2 = vec_dot(body_get_velocity(body2), collision_axis);
  double reduced_mass = ((mass1 * mass2) / (mass1 + mass2));
  if (mass1 == INFINITY) {
    reduced_mass = mass2;
  }
  if (mass2 == INFINITY) {
    reduced_mass = mass1;
  }
  vector_t impulse = vec_multiply(
      reduced_mass * (1 + aux_h->force_const) * (u2 - u1), collision_axis);
  body_add_impulse(body1, impulse);
  body_add_impulse(body2, vec_negate(impulse));
}

void create_angular_collision(scene_t *scene, double elasticity, body_t *body1,
                              body_t *body2) {
  aux_t *aux = malloc(sizeof(aux_t));
  aux->bodies = list_init(2, NULL);
  aux->force_const = elasticity;
  list_add(aux->bodies, body1);
  list_add(aux->bodies, body2);
  create_collision(scene, body1, body2, angular_collision_handler, aux,
                   (free_func_t)free_aux);
}

void angular_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                               void *aux) {
  aux_t *aux_h = aux;
  vector_t collision_axis = axis;
  double mass1 = body_get_mass(body1);
  double mass2 = body_get_mass(body2);
  vector_t centroid1 = body_get_centroid(body1);
  vector_t centroid2 = body_get_centroid(body2);
  double u1 = vec_dot(body_get_velocity(body1), collision_axis);
  double ang_vel = body_get_angular_velocity(body2);
  double delta_centroid_x = centroid2.x - centroid1.x;
  double delta_centroid_y = centroid2.y - centroid1.y;
  if (delta_centroid_x > 0) {
    delta_centroid_x *= -1.0;
  }
  if (delta_centroid_y > 0) {
    delta_centroid_x *= -1.0;
  }
  double u2 = (delta_centroid_x * ang_vel * collision_axis.x) +
              (delta_centroid_y * ang_vel * collision_axis.y);
  double reduced_mass = ((mass1 * mass2) / (mass1 + mass2));
  if (mass1 == INFINITY) {
    reduced_mass = mass2;
  }
  if (mass2 == INFINITY) {
    reduced_mass = mass1;
  }
  vector_t impulse = vec_multiply(
      reduced_mass * (1 + aux_h->force_const) * (u2 - u1), collision_axis);
  body_add_impulse(body1, impulse);
  body_add_impulse(body2, vec_negate(impulse));
}