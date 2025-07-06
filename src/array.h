#include "api/core/global.h"
#include "api/core/object.h"

#ifndef ARRAY_H
#define ARRAY_H


typedef struct array_struct {
    object_t** elements;
    size_t     length;
} array_t;

/*
 * Create a new array.
 *
 * @param _length The length of the array.
 * @return The new array.
 */
array_t* array_new(size_t _length);

/*  
 * Free an array.
 *
 * @param _array The array to free.
 */
void array_free(array_t* _array);

/*
 * Get an element from an array.
 *
 * @param _array The array to get the element from.
 * @param _index The index of the element to get.
 * @return The element.
 */
object_t* array_get(array_t* _array, size_t _index);

/*
 * Set an element in an array.
 *
 * @param _array The array to set the element in.
 * @param _index The index of the element to set.
 * @param _element The element to set.
 */
void array_set(array_t* _array, size_t _index, object_t* _element);

/*
 * Get the length of an array.
 *
 * @param _array The array to get the length of.
 * @return The length of the array.
 */
size_t array_length(array_t* _array);

/*
 * Push an element to an array.
 *
 * @param _array The array to push the element to.
 * @param _element The element to push.
 */
void array_push(array_t* _array, object_t* _element);

/*
 * Pop an element from an array.
 *
 * @param _array The array to pop the element from.
 * @return The popped element.
 */
object_t* array_pop(array_t* _array);


#endif