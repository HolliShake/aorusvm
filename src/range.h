#include "api/core/global.h"
#include "api/core/vm.h"
#include "array.h"

#ifndef RANGE_H
#define RANGE_H

typedef struct range_struct {
    long start;
    long end;
    long step;
} range_t;

/*
 * Create a new range.
 *
 * @param _start The start of the range.
 * @param _end The end of the range.
 * @param _step The step of the range.
 * @return The new range.
 */
range_t* range_new(long _start, long _end, long _step);

/*
 * Get the length of the range.
 *
 * @param _range The range.
 * @return The length of the range.
 */
size_t range_length(range_t* _range);

/*
 * Get the element at the given index.
 *
 * @param _range The range.
 * @param _index The index.
 * @return The element at the given index.
 */
object_t* range_get(range_t* _range, size_t _index);

/*
 * Convert the range to an array.
 *
 * @param _range The range.
 * @return The array.
 */
object_t* range_to_array(range_t* _range);

/*
 * Free the range.
 *
 * @param _range The range to free.
 */
void range_free(range_t* _range);


#endif