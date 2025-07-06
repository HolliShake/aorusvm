#ifndef OBJECT_TYPE_H
#define OBJECT_TYPE_H

typedef enum object_type_enum {
    OBJECT_TYPE_INT,
    OBJECT_TYPE_DOUBLE,
    OBJECT_TYPE_BOOL,
    OBJECT_TYPE_STRING,
    OBJECT_TYPE_ARRAY,
    OBJECT_TYPE_OBJECT,
    OBJECT_TYPE_FUNCTION,
    OBJECT_TYPE_NATIVE_FUNCTION,
    OBJECT_TYPE_NULL
} object_type_t;

#define OBJECT_TYPE_INT(object) (object->type == OBJECT_TYPE_INT)
#define OBJECT_TYPE_DOUBLE(object) (object->type == OBJECT_TYPE_DOUBLE)
#define OBJECT_TYPE_NUMBER(object) (OBJECT_TYPE_INT(object) || OBJECT_TYPE_DOUBLE(object))
#define OBJECT_TYPE_BOOL(object) (object->type == OBJECT_TYPE_BOOL)
#define OBJECT_TYPE_STRING(object) (object->type == OBJECT_TYPE_STRING)
#define OBJECT_TYPE_NULL(object) (object->type == OBJECT_TYPE_NULL)
#define OBJECT_TYPE_FUNCTION(object) (object->type == OBJECT_TYPE_FUNCTION)
#define OBJECT_TYPE_NATIVE_FUNCTION(object) (object->type == OBJECT_TYPE_NATIVE_FUNCTION)
#define OBJECT_TYPE_CALLABLE(object) (OBJECT_TYPE_FUNCTION(object) || OBJECT_TYPE_NATIVE_FUNCTION(object))

#endif