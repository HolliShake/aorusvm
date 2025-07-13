#include "error.h"
#include "internal.h"
#include "object.h"
#include "type.h"

#pragma region StringC
char* string_allocate(const char* _str) {
    char* str = (char*) malloc(sizeof(char) * (strlen(_str) + 1)); str[0] = '\0';
    ASSERTNULL(str, "failed to allocate memory for string");
    strcpy(str, _str);
    return str;
}

char* string_append_char(char* _dst, char _c) {
    size_t len = strlen(_dst);
    char* new_dst = (char*) realloc(_dst, len + 2); // sizeof(char) is always 1
    ASSERTNULL(new_dst, "failed to allocate memory for string");
    new_dst[len] = _c;
    new_dst[len + 1] = '\0';
    return new_dst;
}

char* string_append(char* _dst, char* _src) {
    size_t dst_len = strlen(_dst);
    size_t src_len = strlen(_src);
    
    _dst = (char*) realloc(_dst, sizeof(char) * (dst_len + src_len + 1));
    ASSERTNULL(_dst, "failed to allocate memory for string");
    
    // Use memcpy instead of strcat for better performance
    memcpy(_dst + dst_len, _src, src_len + 1);
    
    return _dst;
}

char* string_format(char* _format, ...) {
    va_list args;
    va_start(args, _format);
    va_list args_copy;
    va_copy(args_copy, args);
    int size = vsnprintf(NULL, 0, _format, args);
    va_end(args);
    char* str = (char*) malloc(sizeof(char) * (size + 1));
    ASSERTNULL(str, "failed to allocate memory for string");
    vsnprintf(str, size + 1, _format, args_copy);
    va_end(args_copy);
    return str;
}

bool string_is_number(char* _str) {
    if (_str == NULL) return false;

    // Optional: skip leading whitespace
    while (isspace((unsigned char)*_str)) _str++;
    if (*_str == '\0') return false;

    char* endptr = NULL;
    errno = 0;
    double val = strtod(_str, &endptr);

    // Optional: skip trailing whitespace
    while (isspace((unsigned char)*endptr)) endptr++;

    if (endptr != _str && *endptr == '\0' && errno == 0 && !isinf(val) && !isnan(val)) {
        return true;
    }
    return false;
}
#pragma endregion

#pragma region NumberC
int number_coerce_to_int(object_t* _obj) {
    switch (_obj->type) {
        case OBJECT_TYPE_INT:
            return _obj->value.i32;
        case OBJECT_TYPE_DOUBLE:
            return (int) _obj->value.f64;
        case OBJECT_TYPE_STRING:
            if (!string_is_number((char*) _obj->value.opaque)) break;
            return (int) strtol((char*) _obj->value.opaque, NULL, 10);
        default:
            break;
    }
    return 0;
}

long number_coerce_to_long(object_t* _obj) {
    switch (_obj->type) {
        case OBJECT_TYPE_INT:
            return (long) _obj->value.i32;
        case OBJECT_TYPE_DOUBLE:
            return (long) _obj->value.f64;
        case OBJECT_TYPE_STRING:
            if (!string_is_number((char*) _obj->value.opaque)) break;
            return (long) strtol((char*) _obj->value.opaque, NULL, 10);
        default:
            break;
    }
    return 0;
}

double number_coerce_to_double(object_t* _obj) {
    switch (_obj->type) {
        case OBJECT_TYPE_INT:
            return (double) _obj->value.i32;
        case OBJECT_TYPE_DOUBLE:
            return (double) _obj->value.f64;
        case OBJECT_TYPE_STRING:
            if (!string_is_number((char*) _obj->value.opaque)) break;
            return strtod((char*) _obj->value.opaque, NULL);
        default:
            break;
    }
    return 0.0;
}
#pragma endregion

#pragma region PathC
char* path_get_file_name(char* _path) {
    const char* last_sep = strrchr(_path, PATH_SEPARATOR);
    const char* base = last_sep ? last_sep + 1 : _path;

    // Find and remove extension (if any)
    const char* dot = strrchr(base, '.');
    size_t len = dot ? (size_t)(dot - base) : strlen(base);

    char* result = malloc(len + 1);
    strncpy(result, base, len);
    result[len] = '\0';

    return result;
}
#pragma endregion


#pragma region HashC
size_t hash64(char* _str) {
    size_t hash = 5381;
    int c;
    while ((c = *_str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}
#pragma endregion