#include "array.h"
#include <stdlib.h>
#include <stdio.h>  // for PD()

// If you don't have a PD() macro, define it:
#ifndef PD
#define PD(msg) fprintf(stderr, "%s\n", msg)
#endif

array_t* array_new(size_t _capacity) {
    array_t* array = (array_t*) malloc(sizeof(array_t));
    if (!array) return NULL;

    array->elements = (object_t**) calloc(_capacity + 1, sizeof(object_t*));
    if (!array->elements) {
        free(array);
        return NULL;
    }

    array->length   = 0;
    array->capacity = _capacity + 1;
    return array;
}

array_t* array_new_initialized(size_t _capacity) {
    array_t* array = (array_t*) malloc(sizeof(array_t));
    if (!array) return NULL;

    array->elements = (object_t**) calloc(_capacity + 1, sizeof(object_t*));
    if (!array->elements) {
        free(array);
        return NULL;
    }
    array->elements[0] = NULL;

    array->length   = _capacity;
    array->capacity = _capacity + 1;
    return array;
}

void array_free(array_t* _array) {
    if (!_array) return;
    free(_array->elements);
    free(_array);
}

object_t* array_get(array_t* _array, size_t _index) {
    if (!_array || _index < 0 || _index >= _array->length) {
        PD("index out of bounds");
        return NULL;
    }
    return _array->elements[_index];
}

void array_set(array_t* _array, size_t _index, object_t* _element) {
    if (!_array || _index < 0 || (_index >= _array->length && _index != 0)) {
        PD("index out of bounds %zu ~ %zu", _index, _array->length);
        return;
    }
    _array->elements[_index] = _element;
}

size_t array_length(array_t* _array) {
    return _array ? _array->length : 0;
}

void array_push(array_t* _array, object_t* _element) {
    if (!_array) return;

    // Check if resize is needed
    if (_array->length + 1 >= _array->capacity) {
        size_t old_capacity = _array->capacity;
        size_t new_capacity = (old_capacity == 0) ? 4 : old_capacity * 2;

        object_t** new_elements = (object_t**) realloc(_array->elements, sizeof(object_t*) * new_capacity);
        if (!new_elements) {
            PD("failed to allocate memory for array push");
            return;
        }

        // Initialize new slots to NULL
        for (size_t i = old_capacity; i < new_capacity; ++i) {
            new_elements[i] = NULL;
        }

        _array->elements = new_elements;
        _array->capacity = new_capacity;
    }

    _array->elements[_array->length++] = _element;
}

object_t* array_pop(array_t* _array) {
    if (!_array || _array->length == 0) {
        PD("cannot pop from empty array");
        return NULL;
    }
    return _array->elements[--_array->length];
}

void array_extend(array_t* _array, array_t* _other_array) {
    if (!_array || !_other_array) return;

    size_t required = _array->length + _other_array->length;
    if (required > _array->capacity) {
        size_t new_capacity = (_array->capacity == 0) ? required : _array->capacity;
        while (new_capacity < required) new_capacity *= 2;

        object_t** new_elements = realloc(_array->elements, new_capacity * sizeof(object_t*));
        if (!new_elements) {
            PD("failed to allocate memory for array extend");
            return;
        }
        _array->elements = new_elements;
        _array->capacity = new_capacity;
    }

    for (size_t i = 0; i < _other_array->length; i++) {
        _array->elements[_array->length + i] = _other_array->elements[i];
    }

    _array->length += _other_array->length;
}
