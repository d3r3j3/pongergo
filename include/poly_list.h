#ifndef __POLY_LIST_H__
#define __POLY_LIST_H__

#include "vec_list.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * A growable array of polygon vector lists, stored as pointers to malloc()ed
 * vector lists. A list owns all the vector lists within it, so it is
 * responsible for free()ing them. This line does two things:
 * - Declares a "struct poly_list" type
 * - Makes "poly_list_t" an alias for "struct poly_list"
 *
 * You will need to implement this struct type in poly_list.c.
 */
typedef struct poly_list poly_list_t;

/**
 * Allocates memory for a new array with space for the given number of elements.
 * The list is initially empty.
 * Asserts that the required memory was allocated.
 *
 * @param initial_size the number of list elements to allocate space for
 * @return a pointer to the newly allocated list
 */
poly_list_t *poly_list_init(size_t initial_size);

/**
 * Releases the memory allocated for a list.
 *
 * @param list a pointer to a list returned from poly_list_init()
 */
void poly_list_free(poly_list_t *list);

/**
 * Frees the memory allocated for the array.
 *
 * @param list a pointer to a list returned from poly_list_init()
 */
void poly_list_array_free(poly_list_t *list);

/**
 * Reallocates memory for a larger array to compensate for more vec lists
 * being added.
 * @param list a pointer to a list returned from poly_list_init()
 */
void poly_list_resize(poly_list_t *list);

/**
 * Gets the size of a list (the number of occupied elements).
 * Note that this is NOT the list's capacity.
 *
 * @param list a pointer to a list returned from poly_list_init()
 * @return the number of vectors in the list
 */
size_t poly_list_size(poly_list_t *list);

/**
 * Gets the element at a given index in a list.
 * Asserts that the index is valid, given the list's current size.
 *
 * @param list a pointer to a list returned from poly_list_init()
 * @param index an index in the list (the first element is at 0)
 * @return the vector list at the given index
 */
void *poly_list_get(poly_list_t *list, size_t index);

/**
 * Appends an element to the end of a list.
 * Asserts that the list has has remaining space,
 * and that the value being added is not NULL.
 *
 * @param list a pointer to a list returned from poly_list_init()
 * @param poly the vector to add to the end of the list
 */
void poly_list_add(poly_list_t *list, void *poly);

/**
 * Removes the element at the end of a list and returns it.
 * Asserts that the list is nonempty.
 *
 * @param list a pointer to a list returned from poly_list_init()
 * @return the vector list at the end of the list
 */
void *poly_list_remove(poly_list_t *list);

/**
 * Removes the element at the chosen index and returns it.
 * Asserts that the list is nonempty.
 *
 * @param list a pointer to a list returned from poly_list_init()
 * @param index an index at which to remove the vector list
 * @return the vector list at the end of the list
 */
void *poly_list_remove_index(poly_list_t *list, size_t index);

#endif // #ifndef __POLY_LIST_H__