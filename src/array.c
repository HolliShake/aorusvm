#include "array.h"

array_t* array_new(size_t _length) {
    array_t* array = (array_t*) malloc(sizeof(array_t));
    if (array == NULL) {
        return NULL;
    }
    array->elements = (object_t**) malloc(sizeof(object_t*) * _length);
    if (array->elements == NULL) {
        free(array);
        return NULL;
    }
    array->length = _length;
    return array;
}

void array_free(array_t* _array) {
    if (_array == NULL) return;
    free(_array->elements);
    free(_array);
}

object_t* array_get(array_t* _array, size_t _index) {
    if (_array == NULL || _index >= _array->length) {
        PD("index out of bounds");
        return NULL;
    }
    return _array->elements[_index];
}

void array_set(array_t* _array, size_t _index, object_t* _element) {
    if (_array == NULL || _index >= _array->length) {
        PD("index out of bounds");
        return;
    }
    _array->elements[_index] = _element;
}

size_t array_length(array_t* _array) {
    if (_array == NULL) return 0;
    return _array->length;
}

void array_push(array_t* _array, object_t* _element) {
    if (_array == NULL) return;
    
    // Reallocate with increased capacity
    size_t new_length = _array->length + 1;
    _array->elements = (object_t**) realloc(_array->elements, sizeof(object_t*) * new_length);
    if (_array->elements == NULL) {
        PD("failed to allocate memory for array push");
        return;
    }
    
    _array->elements[_array->length] = _element;
    _array->length = new_length;
}

object_t* array_pop(array_t* _array) {
    if (_array == NULL || _array->length == 0) {
        PD("cannot pop from empty array");
        return NULL;
    }
    
    object_t* element = _array->elements[_array->length - 1];
    _array->length--;
    return element;
}

void array_extend(array_t* _array, array_t* _other_array) {
    if (_array == NULL || _other_array == NULL) return;
    size_t new_length = _array->length + _other_array->length;
    _array->elements = (object_t**) realloc(_array->elements, sizeof(object_t*) * new_length);
    if (_array->elements == NULL) {
        PD("failed to allocate memory for array extend");
        return;
    }
    for (size_t i = 0; i < _other_array->length; i++) {
        _array->elements[_array->length + i] = _other_array->elements[i];
    }
    _array->length = new_length;
}