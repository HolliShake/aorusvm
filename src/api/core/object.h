/**
 * @file object.h
 * @brief The object API (Do not modify this file).
 * @author Philipp Andrew Redondo
 * @date 2025-06-13
 * @version 0.1.0
 * @copyright MIT License
 * @note This file is part of the aorusvm project.
 */

#include "global.h"
#include "code.h"

#ifndef API_CORE_OBJECT_H
#define API_CORE_OBJECT_H

/*
 * Object type.
 */
typedef enum object_type_enum object_type_t;

/*
 * Object.
 */
typedef struct object_struct object_t;

/*
 * Native function.
 * @param _argc The number of arguments.
 */
typedef void (*vm_native_function)(int argc);

/*
 * Create a new object.
 * @param _type The type of the object.
 * @return The new object.
 */
DLLEXPORT object_t* object_new(object_type_t _type);

/*
 * Create a new object object.
 * @return The new object.
 */
DLLEXPORT object_t* object_new_object();

/*
 * Create a new int object.
 * @param _value The value.
 * @return The new object.
 */
DLLEXPORT object_t* object_new_int(int _value);

/*
 * Create a new double object.
 * @param _value The value.
 * @return The new object.
 */
DLLEXPORT object_t* object_new_double(double _value);

/*
 * Create a new string object.
 * @param _value The value.
 * @return The new object.
 */
DLLEXPORT object_t* object_new_string(char* _value);

/*
 * Create a new bool object.
 * @param _value The value.
 * @return The new object.
 */
DLLEXPORT object_t* object_new_bool(bool _value);

/*
 * Create a new null object.
 * @return The new object.
 */
DLLEXPORT object_t* object_new_null();

/*
 * Create a new array object.
 * @param _length The length of the array.
 * @return The new object.
 */
DLLEXPORT object_t* object_new_array(size_t _length);

/*
 * Create a new range object.
 * @param _start The start of the range.
 * @param _end The end of the range.
 * @param _step The step of the range.
 * @return The new object.
 */
DLLEXPORT object_t* object_new_range(long _start, long _end, long _step);

/*
 * Create a new iterator object.
 * @param _obj The object to iterate over.
 * @return The new object.
 */
DLLEXPORT object_t* object_new_iterator(object_t* _obj);

/*
 * Create a new function object.
 * @param _bytecode The bytecode.
 * @return The new object.
 */
DLLEXPORT object_t* object_new_function(code_t* _bytecode);

/*
 * Create a new native function object.
 * @param _param_count The number of parameters.
 * @param _function The function.
 * @return The new object.
 */
DLLEXPORT object_t* object_new_native_function(size_t _param_count, vm_native_function _function);

/*
 * Create a new error object.
 * @param _message The message.
 * @param _vm_error True if the error is a VM error, false otherwise.
 * @return The new object.
 */
DLLEXPORT object_t* object_new_error(void* _message, bool _vm_error);

/*
 * Convert an object to a string.
 * @param _obj The object.
 * @return The string.
 */
DLLEXPORT char* object_to_string(object_t* _obj);

/*
 * Check if an object is truthy.
 * @param _obj The object.
 * @return True if the object is truthy, false otherwise.
 */
DLLEXPORT bool object_is_truthy(object_t* _obj);

/*
 * Check if an object is an error.
 * @param _obj The object.
 * @return True if the object is an error, false otherwise.
 */
DLLEXPORT bool object_is_error(object_t* _obj);

/*
 * Check if an object is a number.
 * @param _obj The object.
 * @return True if the object is a number, false otherwise.
 */
DLLEXPORT bool object_is_number(object_t* _obj);

/*
 * Check if two objects are equal.
 *
 * @param _obj1 The first object.
 * @param _obj2 The second object.
 * @return True if the objects are equal, false otherwise.
 */
DLLEXPORT bool object_equals(object_t* _obj1, object_t* _obj2);

/*
 * Convert an object type to a string.
 * @param _type The type.
 * @return The string.
 */
DLLEXPORT char* object_type_to_string(object_t* _obj);

/*
 * Get the hash of the object.
 *
 * @param _obj The object.
 * @return The hash of the object.
 */
DLLEXPORT uint64_t object_hash(object_t* _obj);

#endif