#include "api/core/global.h"

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
 * Free the range.
 *
 * @param _range The range to free.
 */
void range_free(range_t* _range);


#endif