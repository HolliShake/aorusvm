#include "api/core/object.h"
#include "type.h"

#ifndef OBJECT_H
#define OBJECT_H

typedef struct object_struct {
    object_type_t type;
    union object_union {
        int    i32;
        long   i64;
        float  f32;
        double f64;
        void   *opaque;
    } value;
    // for garbage collection
    bool marked;
    object_t *next;
} object_t;

#endif