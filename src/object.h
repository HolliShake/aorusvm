#include "api/core/object.h"
#include "api/core/vm.h"
#include "array.h"
#include "code.h"
#include "hashmap.h"
#include "iterator.h"
#include "range.h"
#include "type.h"

#ifndef OBJECT_H
#define OBJECT_H

typedef struct object_struct {
    object_type_t type;
    union object_union {
        int    i32;
        double f64;
        void*  opaque;
    } value;
    // for garbage collection
    bool      marked;
    object_t* next;
} object_t;

typedef struct user_type_struct {
    char*     name;
    object_t* super;
    object_t* prototype;
} user_type_t;

typedef struct user_type_instance_struct {
    object_t* constructor;
    object_t* object;
} user_type_instance_t;

// NOTE: This is a special object type that is used to create user-defined types.
// It is not a regular object type, it is a special object type that is used to create user-defined types.
// It is not a regular object type, it is a special object type that is used to create user-defined types.
object_t* object_new_user_type(char* _name, object_t* _super, object_t* _prototype);

// NOTE: This is a special object type that is used to create user-defined types.
// It is not a regular object type, it is a special object type that is used to create user-defined types.
// It is not a regular object type, it is a special object type that is used to create user-defined types.
object_t* object_new_user_type_instance(object_t* _constructor, object_t* _object);

#endif