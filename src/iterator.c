#include "iterator.h"
#include "object.h"

iterator_t* iterator_new(object_t* _obj) {
    iterator_t* iterator = malloc(sizeof(iterator_t));
    ASSERTNULL(iterator, "failed to allocate memory for iterator");
    iterator->obj = _obj;
    iterator->next = NULL;

    if (OBJECT_TYPE_ARRAY(_obj)) {
        array_t* array = (array_t*) _obj->value.opaque;
        iterator->start = 0;
        iterator->end = array->length;
        iterator->step = 1;
    } else if (OBJECT_TYPE_RANGE(_obj)) {
        range_t* range = (range_t*) _obj->value.opaque;
        iterator->start = range->start;
        iterator->end = range->end;
        iterator->step = range->step;
    } else if (OBJECT_TYPE_OBJECT(_obj)) {
        hashmap_t* hashmap = (hashmap_t*) _obj->value.opaque;
        iterator->start = 0;
        iterator->end = hashmap->bucket_count;
        iterator->step = 1;
        // Find first non-empty bucket
        while (iterator->start < iterator->end && hashmap->buckets[iterator->start] == NULL) {
            iterator->start++;
        }
        iterator->next = iterator->start < iterator->end ? hashmap->buckets[iterator->start] : NULL;
    } else {
        PD("not supported");
    }

    return iterator;
}

bool iterator_has_next(object_t* _obj) {
    iterator_t* iterator = (iterator_t*) _obj->value.opaque;
    
    if (OBJECT_TYPE_ARRAY(iterator->obj) || OBJECT_TYPE_RANGE(iterator->obj)) {
        return iterator->start < iterator->end;
    } else if (OBJECT_TYPE_OBJECT(iterator->obj)) {
        return iterator->next != NULL;
    }
    PD("not supported type: %s", object_type_to_string(iterator->obj));
    return false;
}

object_t** iterator_next(object_t* _obj) {
    iterator_t* iterator = (iterator_t*) _obj->value.opaque;
    static object_t* values[2];
    
    if (OBJECT_TYPE_ARRAY(iterator->obj)) {
        array_t* array = (array_t*) iterator->obj->value.opaque;
        values[0] = array_get(array, iterator->start++);
        values[1] = NULL;
        return values;
    } else if (OBJECT_TYPE_RANGE(iterator->obj)) {
        values[0] = range_get((range_t*)iterator->obj->value.opaque, iterator->start++);
        values[1] = NULL;
        return values;
    } else if (OBJECT_TYPE_OBJECT(iterator->obj)) {
        hashmap_t* hashmap = (hashmap_t*) iterator->obj->value.opaque;
        hashmap_node_t* node = (hashmap_node_t*)iterator->next;
        
        values[0] = node->key;
        values[1] = node->value;
        
        // Update next pointer
        iterator->next = node->next;
        if (iterator->next == NULL) {
            iterator->start++;
            // Find next non-empty bucket
            while (iterator->start < iterator->end && hashmap->buckets[iterator->start] == NULL) {
                iterator->start++;
            }
            if (iterator->start < iterator->end) {
                iterator->next = hashmap->buckets[iterator->start];
            }
        }
        
        return values;
    }
    
    PD("not supported");
    return NULL;
}

void iterator_free(iterator_t* _iterator) {
    free(_iterator);
}