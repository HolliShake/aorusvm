#include "api/core/object.h"
#include "api/core/vm.h"
#include "array.h"
#include "code.h"
#include "type.h"

#ifndef OBJECT_H
#define OBJECT_H

typedef struct object_struct {
    object_type_t type;
    union object_union {
        int    i32;
        double f64;
        void   *opaque;
    } value;
    // for garbage collection
    bool marked;
    object_t *next;
} object_t;

#endif