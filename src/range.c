#include "range.h"

range_t* range_new(long _start, long _end, long _step) {
    range_t* range = (range_t*) malloc(sizeof(range_t));
    if (range == NULL) {
        PD("failed to allocate memory for range");
    }
    range->start = _start;
    range->end = _end;
    range->step = _step;
    return range;
}

void range_free(range_t* _range) {
    free(_range);
}