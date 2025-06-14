#ifndef OBJECT_TYPE_H
#define OBJECT_TYPE_H

typedef enum object_type_enum {
    OBJECT_TYPE_INT,
    OBJECT_TYPE_LONG,
    OBJECT_TYPE_FLOAT,
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
#define OBJECT_TYPE_LONG(object) (object->type == OBJECT_TYPE_LONG)
#define OBJECT_TYPE_LONG_INT(object) (OBJECT_TYPE_LONG(object) || OBJECT_TYPE_INT(object))
#define OBJECT_TYPE_FLOAT(object) (object->type == OBJECT_TYPE_FLOAT)
#define OBJECT_TYPE_DOUBLE(object) (object->type == OBJECT_TYPE_DOUBLE)
#define OBJECT_TYPE_DOUBLE_FLOAT(object) (OBJECT_TYPE_DOUBLE(object) || OBJECT_TYPE_FLOAT(object))
#define OBJECT_TYPE_NUMBER(object) (OBJECT_TYPE_INT(object) || OBJECT_TYPE_LONG(object) || OBJECT_TYPE_FLOAT(object) || OBJECT_TYPE_DOUBLE(object))
#define OBJECT_TYPE_BOOL(object) (object->type == OBJECT_TYPE_BOOL)
#define OBJECT_TYPE_STRING(object) (object->type == OBJECT_TYPE_STRING)
#define OBJECT_TYPE_NULL(object) (object->type == OBJECT_TYPE_NULL)

#endif