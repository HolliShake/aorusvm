#include "api/core/object.h"
#include "error.h"
#include "internal.h"
#include "object.h"
#include "type.h"

DLLEXPORT object_t *object_new(object_type_t _type) {
    object_t *obj = (object_t *) malloc(sizeof(object_t));
    ASSERTNULL(obj, ERROR_ALLOCATING_OBJECT);
    obj->type = _type;
    obj->next = NULL;
    obj->marked = false;
    return obj;
}

DLLEXPORT object_t *object_new_int(int _value) {
    object_t *obj = object_new(OBJECT_TYPE_INT);
    obj->value.i32 = _value;
    return obj;
}

DLLEXPORT object_t *object_new_double(double _value) {
    object_t *obj = object_new(OBJECT_TYPE_DOUBLE);
    obj->value.f64 = _value;
    return obj;
}

DLLEXPORT object_t *object_new_bool(bool _value) {
    object_t *obj = object_new(OBJECT_TYPE_BOOL);
    obj->value.i32 = _value;
    return obj;
}

DLLEXPORT object_t *object_new_string(char *_value) {
    object_t *obj = object_new(OBJECT_TYPE_STRING);
    obj->value.opaque = string_allocate((const char*) _value);
    return obj;
}

DLLEXPORT object_t *object_new_null() {
    object_t *obj = object_new(OBJECT_TYPE_NULL);
    return obj;
}

DLLEXPORT char* object_to_string(object_t *_obj) {
    char str[255];
    switch (_obj->type) {
        case OBJECT_TYPE_INT:
            sprintf(str, "%d", _obj->value.i32);
            return string_allocate(str);
        case OBJECT_TYPE_DOUBLE: {
            double intpart;
            if (modf(_obj->value.f64, &intpart) == 0.0) {
                sprintf(str, "%d", (int) _obj->value.f64);
            } else {
                sprintf(str, "%.2lf", (double) _obj->value.f64);
            }
            return string_allocate(str);
        }
        case OBJECT_TYPE_STRING: {
            return string_allocate((char*) _obj->value.opaque);
        }
        case OBJECT_TYPE_BOOL: {
            strcpy(str, _obj->value.i32 ? "true" : "false");
            return string_allocate(str);
        }
        case OBJECT_TYPE_NULL: {
            strcpy(str, "null");
            return string_allocate(str);
        }
        default: {
            strcpy(str, "unknown");
            return string_allocate(str);
        }
    }
}

DLLEXPORT bool object_is_truthy(object_t *_obj) {
    switch (_obj->type) {
        case OBJECT_TYPE_INT:
        case OBJECT_TYPE_DOUBLE:
            return (double) _obj->value.f64 != 0;
        case OBJECT_TYPE_STRING:
            return strlen((char*) _obj->value.opaque) > 0;
        case OBJECT_TYPE_BOOL:
            return _obj->value.i32 != 0;
        case OBJECT_TYPE_NULL:
            return false;
        default:
            return false;
    }
}

DLLEXPORT bool object_is_number(object_t *_obj) {
    switch (_obj->type) {
        case OBJECT_TYPE_INT:
        case OBJECT_TYPE_DOUBLE:
            return true;
        case OBJECT_TYPE_STRING:
            return string_is_number((char*) _obj->value.opaque);
        default:
            return false;
    }
}