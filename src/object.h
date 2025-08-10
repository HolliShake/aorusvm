#include "api/core/async.h"
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

/*
 * Creates a new promise object with the specified async state and value.
 * 
 * Promises are used for asynchronous operations and can be in different states
 * (pending, fulfilled, rejected) as defined by async_state_t.
 * 
 * @param _state The initial async state of the promise
 * @param _value The value associated with the promise (may be NULL for pending promises)
 * @return A new promise object, or NULL if allocation fails
 */
object_t* object_new_promise(async_state_t _state, object_t* _value);

/*
 * Creates a new user-defined type object.
 * 
 * User-defined types allow for custom object types with inheritance and prototypes.
 * This function creates the type definition itself, not an instance of the type.
 * 
 * @param _name The name of the user-defined type (must be non-NULL)
 * @param _super The parent type for inheritance (may be NULL for no inheritance)
 * @param _prototype The prototype object containing default methods and properties
 * @return A new user type object, or NULL if allocation fails
 */
object_t* object_new_user_type(char* _name, object_t* _super, object_t* _prototype);

/*
 * Creates a new instance of a user-defined type.
 * 
 * This function creates an actual instance of a user-defined type, linking it
 * to its constructor and the underlying object data.
 * 
 * @param _constructor The constructor function/object used to create this instance
 * @param _object The underlying object data for this instance
 * @return A new user type instance object, or NULL if allocation fails
 */
object_t* object_new_user_type_instance(object_t* _constructor, object_t* _object);

/*
 * Converts an object to a string representation with indentation.
 * 
 * This function converts an object to a string representation, including
 * indentation for nested structures.
 * 
 */
char* object_object_to_string_with_indent(object_t* _obj, int _indent);

#endif