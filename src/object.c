#include "api/core/object.h"
#include "error.h"
#include "internal.h"
#include "object.h"
#include "type.h"

DLLEXPORT object_t* object_new(object_type_t _type) {
    object_t* obj = (object_t* ) malloc(sizeof(object_t));
    ASSERTNULL(obj, "failed to allocate memory for object");
    obj->type = _type;
    obj->next = NULL;
    obj->marked = false;
    return obj;
}

DLLEXPORT object_t* object_new_object() {
    object_t* obj = object_new(OBJECT_TYPE_OBJECT);
    obj->value.opaque = hashmap_new();
    return obj;
}

DLLEXPORT object_t* object_new_int(int _value) {
    object_t* obj = object_new(OBJECT_TYPE_INT);
    obj->value.i32 = _value;
    return obj;
}

DLLEXPORT object_t* object_new_double(double _value) {
    object_t* obj = object_new(OBJECT_TYPE_DOUBLE);
    obj->value.f64 = _value;
    return obj;
}

DLLEXPORT object_t* object_new_string(char *_value) {
    object_t* obj = object_new(OBJECT_TYPE_STRING);
    obj->value.opaque = string_allocate((const char*) _value);
    return obj;
}

DLLEXPORT object_t* object_new_null() {
    object_t* obj = object_new(OBJECT_TYPE_NULL);
    return obj;
}

DLLEXPORT object_t* object_new_bool(bool _value) {
    object_t* obj = object_new(OBJECT_TYPE_BOOL);
    obj->value.i32 = _value;
    return obj;
}

DLLEXPORT object_t* object_new_array(size_t _length) {
    object_t* obj = object_new(OBJECT_TYPE_ARRAY);
    obj->value.opaque = array_new_initialized(_length);
    return obj;
}

DLLEXPORT object_t* object_new_range(long _start, long _end, long _step) {
    object_t* obj = object_new(OBJECT_TYPE_RANGE);
    obj->value.opaque = range_new(_start, _end, _step);
    return obj;
}

DLLEXPORT object_t* object_new_function(bool _is_async, size_t _param_count, uint8_t* _bytecode, size_t _size) {
    object_t* obj = object_new(OBJECT_TYPE_FUNCTION);
    obj->value.opaque = code_new_function(_is_async, _param_count, _bytecode, _size);
    return obj;
}

DLLEXPORT object_t* object_new_native_function(size_t _param_count, vm_native_function _function) {
    object_t* obj = object_new(OBJECT_TYPE_NATIVE_FUNCTION);
    obj->value.opaque = _function;
    return obj;
}

DLLEXPORT object_t* object_new_error(void* _message, bool _vm_error) {
    object_t* obj = object_new(OBJECT_TYPE_ERROR);
    if (_vm_error) {
        // _message here should be a string a.k.a C string.
        obj->value.opaque = 
            vm_to_heap(object_new_string((char*) _message));
    } else {
        // _message here should be an object_t*, that is already in the heap
        obj->value.opaque = 
            (object_t*) _message;
    }
    return obj;
}

object_t* object_new_iterator(object_t* _obj) {
    object_t* obj = object_new(OBJECT_TYPE_ITERATOR);
    obj->value.opaque = iterator_new(_obj);
    return obj;
}

DLLEXPORT char* object_to_string(object_t* _obj) {
    if (_obj == NULL) return string_allocate("<cnull>");
    switch (_obj->type) {
        case OBJECT_TYPE_INT:
            return string_format("%d", _obj->value.i32);
        case OBJECT_TYPE_DOUBLE: {
            double intpart;
            if (modf(_obj->value.f64, &intpart) == 0.0) {
                return string_format("%ld", (long) _obj->value.f64);
            } else {
                return string_format("%.2lf", (double) _obj->value.f64);
            }
        }
        case OBJECT_TYPE_STRING: {
            return string_allocate((char*) _obj->value.opaque);
        }
        case OBJECT_TYPE_BOOL: {
            return string_allocate(_obj->value.i32 ? "true" : "false");
        }
        case OBJECT_TYPE_NULL: {
            return string_allocate("null");
        }
        case OBJECT_TYPE_ARRAY: {
            array_t* array = (array_t*) _obj->value.opaque;
            size_t len = array_length(array);
            
            // Start with a reasonable buffer size
            size_t initial_capacity = 32 + len * 8; // Estimate ~8 chars per element
            char* result = malloc(initial_capacity);
            if (!result) return NULL;
            
            size_t capacity = initial_capacity;
            size_t used = 0;
            
            // Start with opening bracket
            result[used++] = '[';
            
            for (size_t i = 0; i < len; ++i) {
                object_t* element = array_get(array, i);
                
                // Handle self-reference
                if (element == _obj) {
                    const char* self_ref = "<self>";
                    size_t self_len = 6;
                    
                    // Ensure buffer has enough space
                    if (used + self_len + 3 >= capacity) { // +3 for possible ", " and null terminator
                        capacity = capacity * 2;
                        char* new_buf = realloc(result, capacity);
                        if (!new_buf) {
                            free(result);
                            return NULL;
                        }
                        result = new_buf;
                    }
                    
                    memcpy(result + used, self_ref, self_len);
                    used += self_len;
                } else {
                    // Get string representation of element
                    char* str = object_to_string(element);
                    if (!str) {
                        free(result);
                        return NULL;
                    }
                    
                    size_t str_len = strlen(str);
                    
                    // Ensure buffer has enough space
                    if (used + str_len + 3 >= capacity) { // +3 for possible ", " and null terminator
                        capacity = capacity * 2 + str_len;
                        char* new_buf = realloc(result, capacity);
                        if (!new_buf) {
                            free(str);
                            free(result);
                            return NULL;
                        }
                        result = new_buf;
                    }
                    
                    memcpy(result + used, str, str_len);
                    used += str_len;
                    free(str);
                }
                
                // Add separator if not the last element
                if (i < len - 1) {
                    result[used++] = ',';
                    result[used++] = ' ';
                }
            }
            
            // Add closing bracket and null terminator
            result[used++] = ']';
            result[used] = '\0';
            
            // Trim excess memory if significantly oversized
            if (capacity > used + 128) {
                char* trimmed = realloc(result, used + 1);
                if (trimmed) result = trimmed;
            }
            
            return result;
        }
        case OBJECT_TYPE_RANGE: {
            range_t* range = (range_t*) _obj->value.opaque;
            return string_format("range(%ld, %ld, %ld)", range->start, range->end, range->step);
        }
        case OBJECT_TYPE_OBJECT: {
            hashmap_t* map = (hashmap_t*) _obj->value.opaque;
            size_t entries_count = hashmap_size(map);
            
            if (entries_count == 0) {
                return string_allocate("{}");
            }
            
            // Pre-allocate buffer with estimated size
            size_t capacity = 32 + entries_count * 16; // Initial estimate
            char* result = malloc(capacity);
            if (!result) return NULL;
            
            size_t used = 0;
            result[used++] = '{';
            
            size_t entries_added = 0;
            for (size_t i = 0; i < map->bucket_count; i++) {
                hashmap_node_t* node = map->buckets[i];
                while (node) {
                    // Add separator if not the first element
                    if (entries_added > 0) {
                        if (used + 2 >= capacity) {
                            capacity *= 2;
                            char* new_buf = realloc(result, capacity);
                            if (!new_buf) {
                                free(result);
                                return NULL;
                            }
                            result = new_buf;
                        }
                        result[used++] = ',';
                        result[used++] = ' ';
                    }
                    
                    // Get string representation of key
                    char* key_str = object_to_string(node->key);
                    if (!key_str) {
                        free(result);
                        return NULL;
                    }
                    
                    size_t key_len = strlen(key_str);
                    
                    // Ensure buffer has enough space for key
                    if (used + key_len + 2 >= capacity) {
                        capacity = capacity * 2 + key_len;
                        char* new_buf = realloc(result, capacity);
                        if (!new_buf) {
                            free(key_str);
                            free(result);
                            return NULL;
                        }
                        result = new_buf;
                    }
                    
                    // Copy key string
                    memcpy(result + used, key_str, key_len);
                    used += key_len;
                    free(key_str);
                    
                    // Add ": " separator
                    if (used + 2 >= capacity) {
                        capacity *= 2;
                        char* new_buf = realloc(result, capacity);
                        if (!new_buf) {
                            free(result);
                            return NULL;
                        }
                        result = new_buf;
                    }
                    result[used++] = ':';
                    result[used++] = ' ';
                    
                    // Get string representation of value
                    char* val_str = object_to_string(node->value);
                    if (!val_str) {
                        free(result);
                        return NULL;
                    }
                    
                    size_t val_len = strlen(val_str);
                    
                    // Ensure buffer has enough space for value
                    if (used + val_len >= capacity) {
                        capacity = capacity * 2 + val_len;
                        char* new_buf = realloc(result, capacity);
                        if (!new_buf) {
                            free(val_str);
                            free(result);
                            return NULL;
                        }
                        result = new_buf;
                    }
                    
                    // Copy value string
                    memcpy(result + used, val_str, val_len);
                    used += val_len;
                    free(val_str);
                    
                    entries_added++;
                    node = node->next;
                }
            }
            
            // Add closing brace and null terminator
            if (used + 1 >= capacity) {
                char* new_buf = realloc(result, used + 2);
                if (!new_buf) {
                    free(result);
                    return NULL;
                }
                result = new_buf;
            }
            result[used++] = '}';
            result[used] = '\0';
            
            return result;
        }
        case OBJECT_TYPE_FUNCTION: {
            char* str = string_allocate("function");
            str = string_append(str, "(");
            code_t* code = (code_t*) _obj->value.opaque;
            for (size_t i = 0; i < code->param_count; i++) {
                char* fmt = string_format("arg%d", i);
                str = string_append(str, fmt);
                free(fmt);
                if (i < code->param_count - 1) {
                    str = string_append(str, ", ");
                }
            }
            str = string_append(str, ")");
            str = string_append(str, "{...}");
            return str;
        }
        case OBJECT_TYPE_NATIVE_FUNCTION: {
            return string_allocate("native function(...){...}");
        }
        case OBJECT_TYPE_ERROR: {
            return string_format("error: %s", object_to_string((object_t*) _obj->value.opaque));
        }
        default: {
            return string_allocate("unknown object");
        }
    }
}

DLLEXPORT bool object_is_truthy(object_t* _obj) {
    switch (_obj->type) {
        case OBJECT_TYPE_INT:
        case OBJECT_TYPE_DOUBLE:
            return number_coerce_to_double(_obj) != 0;
        case OBJECT_TYPE_STRING:
            return strlen((char*) _obj->value.opaque) > 0;
        case OBJECT_TYPE_BOOL:
            return _obj->value.i32 != 0;
        case OBJECT_TYPE_NULL:
            return false;
        case OBJECT_TYPE_ARRAY:
            return array_length((array_t*) _obj->value.opaque) > 0;
        case OBJECT_TYPE_RANGE:
            return true;
        case OBJECT_TYPE_OBJECT:
            return hashmap_size((hashmap_t*) _obj->value.opaque) > 0;
        case OBJECT_TYPE_ERROR:
            return object_is_truthy((object_t*) _obj->value.opaque);
        default:
            return false;
    }
}

DLLEXPORT bool object_is_error(object_t* _obj) {
    return _obj->type == OBJECT_TYPE_ERROR;
}

DLLEXPORT bool object_is_number(object_t* _obj) {
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

DLLEXPORT bool object_equals(object_t* _obj1, object_t* _obj2) {
    if (_obj1->type != _obj2->type) return false;
    switch (_obj1->type) {
        case OBJECT_TYPE_INT:
            return _obj1->value.i32 == _obj2->value.i32;
        case OBJECT_TYPE_DOUBLE:
            return _obj1->value.f64 == _obj2->value.f64;
        case OBJECT_TYPE_STRING:
            return strcmp((char*) _obj1->value.opaque, (char*) _obj2->value.opaque) == 0;
        case OBJECT_TYPE_BOOL:
            return _obj1->value.i32 == _obj2->value.i32;
        case OBJECT_TYPE_NULL:
            return _obj2->type == OBJECT_TYPE_NULL;
        case OBJECT_TYPE_ARRAY: {
            if (_obj1 == _obj2) return true;
            array_t* array1 = (array_t*) _obj1->value.opaque;
            array_t* array2 = (array_t*) _obj2->value.opaque;
            if (array_length(array1) != array_length(array2)) return false;
            for (size_t i = 0; i < array_length(array1); i++) {
                if (!object_equals(array_get(array1, i), array_get(array2, i))) return false;
            }
            return true;
        }
        case OBJECT_TYPE_RANGE: {
            range_t* range1 = (range_t*) _obj1->value.opaque;
            range_t* range2 = (range_t*) _obj2->value.opaque;
            return range1->start == range2->start && range1->end == range2->end && range1->step == range2->step;
        }
        case OBJECT_TYPE_OBJECT: {
            hashmap_t* map1 = (hashmap_t*) _obj1->value.opaque;
            hashmap_t* map2 = (hashmap_t*) _obj2->value.opaque;
            if (map1->size != map2->size) return false;
            for (size_t i = 0; i < map1->size; i++) {
                object_t* key = map1->buckets[i]->key;
                object_t* val1 = hashmap_get(map1, key);
                object_t* val2 = hashmap_get(map2, key);
                if (!object_equals(val1, val2)) return false;
            }
            return true;
        }
        case OBJECT_TYPE_FUNCTION:
        case OBJECT_TYPE_NATIVE_FUNCTION:
        case OBJECT_TYPE_ERROR:
            return _obj1->value.opaque == _obj2->value.opaque || _obj1 == _obj2;
        default:
            return _obj1 == _obj2;
    }
}

DLLEXPORT char* object_type_to_string(object_t* _obj) {
    if (_obj == NULL) return "<cnull>";
    switch (_obj->type) {
        case OBJECT_TYPE_INT:
            return "int";
        case OBJECT_TYPE_DOUBLE:
            return "number";
        case OBJECT_TYPE_STRING:
            return "string";
        case OBJECT_TYPE_BOOL:
            return "boolean";
        case OBJECT_TYPE_NULL:
            return "null";
        case OBJECT_TYPE_ARRAY:
        case OBJECT_TYPE_RANGE:
            return "array";
        case OBJECT_TYPE_ITERATOR:
            return string_format("<iterator.%s/>", object_type_to_string(_obj->value.opaque));
        case OBJECT_TYPE_OBJECT:
            return "object";
        case OBJECT_TYPE_FUNCTION:
            return "function";
        case OBJECT_TYPE_NATIVE_FUNCTION:
            return "native function";
        case OBJECT_TYPE_ERROR:
            return "error";
        default:
            return string_format("<unknown.%d/>", _obj->type);
    }
}

DLLEXPORT size_t object_hash(object_t* _obj) {
    switch (_obj->type) {
        case OBJECT_TYPE_INT:
            return (size_t) _obj->value.i32;
        case OBJECT_TYPE_DOUBLE: {
            double intpart;
            if (modf(_obj->value.f64, &intpart) == 0.0) {
                // int or long
                return (size_t) _obj->value.f64;
            } else {
                // double
                #if IS_64BIT
                    uint64_t bits;
                    uint64_t hash = 14695981039346656037ull;
                    uint64_t prime = 1099511628211ull;
                #else
                    uint32_t bits;
                    uint32_t hash = 2166136261u;
                    uint32_t prime = 16777619u;
                #endif
                memcpy(&bits, &_obj->value.f64, sizeof(double));

                for (int i = 0; i < 8;i++) {
                    hash ^= (bits >> (i * 8)) & 0xFF;
                    hash *= prime;
                }
                return (size_t) hash;
            }
        }
        case OBJECT_TYPE_STRING:
            return (size_t) hash64((char*) _obj->value.opaque);
        case OBJECT_TYPE_BOOL:
            return (size_t) _obj->value.i32;
        case OBJECT_TYPE_NULL:
            return 0;
        case OBJECT_TYPE_ARRAY: {
            #if IS_64BIT
                size_t hash = 14695981039346656037u;  // FNV offset basis (Fowler–Noll–Vo)
                size_t prime = 1099511628211u; // FNV prime
            #else
                size_t hash = 2166136261u;  // FNV offset basis (Fowler–Noll–Vo)
                size_t prime = 16777619u; // FNV prime
            #endif
            array_t* array = (array_t*) _obj->value.opaque;
            for (size_t i = 0; i < array_length(array); i++) {
                object_t* element = array_get(array, i);
                hash ^= object_hash(element);
                hash *= prime; // FNV prime
            }
            return (size_t) hash;
        }
        case OBJECT_TYPE_RANGE: {
            range_t* range = (range_t*) _obj->value.opaque;
            return (size_t) _obj->value.opaque ^ range->start ^ range->end ^ range->step;
        }
        case OBJECT_TYPE_OBJECT: {
            #if IS_64BIT
                size_t hash = 14695981039346656037u;  // FNV offset basis (Fowler–Noll–Vo)
                size_t prime = 1099511628211u; // FNV prime
            #else
                size_t hash = 2166136261u;  // FNV offset basis (Fowler–Noll–Vo)
                size_t prime = 16777619u; // FNV prime
            #endif
            hashmap_t* map = (hashmap_t*) _obj->value.opaque;
            for (size_t i = 0; i < map->size; i++) {
                object_t* key = map->buckets[i]->key;
                hash ^= object_hash(key);
                hash *= prime; // FNV prime
            }
            return (size_t) hash;
        }
        case OBJECT_TYPE_FUNCTION:
            return (size_t) _obj->value.opaque;
        case OBJECT_TYPE_NATIVE_FUNCTION:
            return (size_t) _obj->value.opaque;
        case OBJECT_TYPE_ERROR:
            return object_hash((object_t*) _obj->value.opaque);
        default:
            PD("unsupported object type for hash: %d", _obj->type);
            return 0;
    }
}