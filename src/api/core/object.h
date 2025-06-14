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
 * Create a new object.
 * @param _type The type of the object.
 * @return The new object.
 */
DLLEXPORT object_t *object_new(object_type_t _type);

/*
 * Create a new int object.
 * @param _value The value.
 * @return The new object.
 */
DLLEXPORT object_t *object_new_int(int _value);

/*
 * Create a new long object.
 * @param _value The value.
 * @return The new object.
 */
DLLEXPORT object_t *object_new_long(long _value);

/*
 * Create a new float object.
 * @param _value The value.
 * @return The new object.
 */
DLLEXPORT object_t *object_new_float(float _value);

/*
 * Create a new double object.
 * @param _value The value.
 * @return The new object.
 */
DLLEXPORT object_t *object_new_double(double _value);

/*
 * Create a new bool object.
 * @param _value The value.
 * @return The new object.
 */
DLLEXPORT object_t *object_new_bool(bool _value);

/*
 * Create a new string object.
 * @param _value The value.
 * @return The new object.
 */
DLLEXPORT object_t *object_new_string(char* _value);

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

#endif