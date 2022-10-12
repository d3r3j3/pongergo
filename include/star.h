#ifndef __STAR_H__
#define __STAR_H__

#include "color.h"
#include "polygon.h"

/**
 * An array of vectors stored as pointers, that form to complete a n-pointed
 * star with a defined inner and outer radius, amount of points, velocity,
 * angular velocity, elasticity, centroid, and weight.
 */
typedef struct star star_t;

/**
 * Allocates memory for a new vec_list with space given for the amount of points
 * needed.
 * The list is initially empty.
 * Asserts that the required memory was allocated
 *
 * @param num_points the number of points wanted for the star
 * @return a pointer to the newly allocated list.
 */
star_t *star_t_init(size_t star_points);

/**
 * Makes the star and constructs most of its physical properties including
 *color, shape, size, and weight.
 *
 * @param list a pointer to a star returned from star_t_init,
 * i_rad and o_rad are used to construct the size of the star,
 * w is the weight of the star, elasticity is the elastcity of the star,
 * r, g, and b, are used to decide the color of the star.
 * @return void.
 **/
void make_star(star_t *star, double i_rad, double o_rad, rgb_color_t color,
               double weight, double elasticity);

/**
 * Rotates the star
 * @param  list a pointer to a star returned from star_t_init, angle, the amount
 *by which the position of the star rotates about its center
 * @return void.
 **/
void star_rot(star_t *star, double angle);

/**
 * Gets the vertices that make up the star
 * @param star a pointer to a star returned from star_t_init
 * @return A vector list that pointer that contains the vectors that make up
 * the star
 **/
vec_list_t *star_get_vertices(star_t *star);

/**
 * Gets the velocity vector pointer of the star
 * @param star a pointer to a star returned from star_t_init
 * @return A vector pointer that points to the velocity vector of the star
 **/
vector_t *star_get_velocity(star_t *star);

/**
 * Frees the memory of the star pointer
 * @param star a pointer to a star returned from star_t_init
 * @return void
 **/
void star_free(star_t *star);

/**
 * Gets the centroid of the star
 * @param star a pointer to a star returned from star_t_init
 * @return a vector that is the center of the star
 **/
vector_t star_get_centroid(star_t *star);

/**
 * Gets the inner radius of the star
 * @param star a pointer to a star returned from star_t_init
 * @return A double that is the size of the star's inner radius
 **/
double star_get_in_rad(star_t *star);

/**
 * Gets the outer radius of the star
 * @param star a pointer to a star returned from star_t_init
 * @return A double that is the size of the star's outer radius
 **/
double star_get_out_rad(star_t *star);

/**
 * Gets the weight of the star
 * @param star a pointer to a star returned from star_t_init
 * @return A double that is the weight of the star
 **/
double star_get_weight(star_t *star);

/**
 * Gets the angular velocity of the star
 * @param star a pointer to a star returned from star_t_init
 * @return A double that is the angular velocity of the star
 **/
double star_get_ang_vel(star_t *star);

/**
 * Gets the elasticity of the star
 * @param star a pointer to a star returned from star_t_init
 * @return A double that is the elasticity of the star
 **/
double star_get_elasticity(star_t *star);

/**
 * Gets the rgb color values of the star
 * @param star a pointer to a star returned from star_t_init
 * @return A rgb_color_t that is the rgb color values of the star
 **/
rgb_color_t star_get_color(star_t *star);

#endif // #ifndef __STAR_H__