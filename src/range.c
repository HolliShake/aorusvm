#include "range.h"
#include "object.h"

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

size_t range_length(range_t* _range) {
    if (_range->step == 0) return 0; // invalid
    if ((_range->step > 0 && _range->start >= _range->end) || (_range->step < 0 && _range->start <= _range->end)) return 0;
    return (size_t)((_range->end - _range->start + _range->step - (_range->step > 0 ? 1 : -1)) / _range->step);
}

object_t* range_get(range_t* _range, size_t _index) {
    if (_index < 0 || _index >= range_length(_range)) {
        return NULL;
    }
    return vm_to_heap(object_new_double(_range->start + _index * _range->step));
}

object_t* range_to_array(range_t* _range) {
    size_t length = range_length(_range);
    if (length == 0) {
        return vm_to_heap(object_new_array(0));
    }
    
    // Create pre-initialized array to avoid repeated resizing
    array_t* arr = array_new_initialized(length);
    if (!arr) return NULL;
    
    object_t* array = vm_to_heap(object_new(OBJECT_TYPE_ARRAY));
    if (!array) {
        array_free(arr);
        return NULL;
    }
    array->value.opaque = arr;
    
    // Pre-allocate all number objects at once
    object_t** items = malloc(length * sizeof(object_t*));
    if (!items) {
        // Don't free array since it's managed by VM
        return NULL;
    }
    
    double start = _range->start;
    double step = _range->step;
    
    // Batch create all objects first
    for (size_t i = 0; i < length; ++i) {
        double value = start + (double)i * step;
        items[i] = vm_to_heap(object_new_double(value));
    }
    
    // Batch copy pointers directly to array elements
    memcpy(arr->elements, items, length * sizeof(object_t*));
    free(items);
    
    return array;
}

void range_free(range_t* _range) {
    free(_range);
}

/*
real    0m0.506s
user    0m0.062s
sys     0m0.034s
*/