#ifndef OBJECT_TYPE_H
#define OBJECT_TYPE_H

typedef enum object_type_enum {
    OBJECT_TYPE_INT,
    OBJECT_TYPE_DOUBLE,
    OBJECT_TYPE_BOOL,
    OBJECT_TYPE_STRING,
    OBJECT_TYPE_ARRAY,
    OBJECT_TYPE_RANGE,
    OBJECT_TYPE_OBJECT,
    OBJECT_TYPE_PROMISE,
    OBJECT_TYPE_USER_TYPE,
    OBJECT_TYPE_USER_TYPE_INSTANCE,
    OBJECT_TYPE_FUNCTION,
    OBJECT_TYPE_NATIVE_FUNCTION,
    OBJECT_TYPE_NULL,
    OBJECT_TYPE_ERROR,
    OBJECT_TYPE_ITERATOR
} object_type_t;

#define OBJECT_TYPE_INT(object) (object->type == OBJECT_TYPE_INT)
#define OBJECT_TYPE_DOUBLE(object) (object->type == OBJECT_TYPE_DOUBLE)
#define OBJECT_TYPE_NUMBER(object) (OBJECT_TYPE_INT(object) || OBJECT_TYPE_DOUBLE(object))
#define OBJECT_TYPE_BOOL(object) (object->type == OBJECT_TYPE_BOOL)
#define OBJECT_TYPE_STRING(object) (object->type == OBJECT_TYPE_STRING)
#define OBJECT_TYPE_NULL(object) (object->type == OBJECT_TYPE_NULL)
#define OBJECT_TYPE_ARRAY(object) (object->type == OBJECT_TYPE_ARRAY)
#define OBJECT_TYPE_RANGE(object) (object->type == OBJECT_TYPE_RANGE)
#define OBJECT_TYPE_USER_TYPE(object) (object->type == OBJECT_TYPE_USER_TYPE)
#define OBJECT_TYPE_USER_TYPE_INSTANCE(object) (object->type == OBJECT_TYPE_USER_TYPE_INSTANCE)
#define OBJECT_TYPE_FUNCTION(object) (object->type == OBJECT_TYPE_FUNCTION)
#define OBJECT_TYPE_OBJECT(object) (object->type == OBJECT_TYPE_OBJECT)
#define OBJECT_TYPE_PROMISE(object) (object->type == OBJECT_TYPE_PROMISE)
#define OBJECT_TYPE_COLLECTION(object) (OBJECT_TYPE_ARRAY(object) || OBJECT_TYPE_OBJECT(object) || OBJECT_TYPE_RANGE(object))
#define OBJECT_TYPE_NATIVE_FUNCTION(object) (object->type == OBJECT_TYPE_NATIVE_FUNCTION)
#define OBJECT_TYPE_CALLABLE(object) (OBJECT_TYPE_FUNCTION(object) || OBJECT_TYPE_NATIVE_FUNCTION(object))
#define OBJECT_TYPE_ERROR(object) (object->type == OBJECT_TYPE_ERROR)
#define OBJECT_TYPE_ITERATOR(object) (object->type == OBJECT_TYPE_ITERATOR)

#endif