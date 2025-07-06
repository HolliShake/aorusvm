/**
 * @file env.h
 * @brief Environment API (Do not modify this file).
 * @author Philipp Andrew Redondo
 * @date 2025-06-13
 * @version 0.1.0
 * @copyright MIT License
 * @note This file is part of the aorusvm project.
 */


#include "global.h"
#include "object.h"

#ifndef ENV_H
#define ENV_H

/*
 * Environment.
 */
typedef struct env_struct env_t;

/*
 * Create a new environment.
 * @return The new environment.
 */
DLLEXPORT env_t* env_new(env_t* _parent);

/*
 * Check if a variable exists in the environment.
 * @param _env The environment.
 * @param _name The name of the variable.
 * @return True if the variable exists, false otherwise.
 */
DLLEXPORT bool env_has(env_t* _env, char* _name, bool _recurse);

/*
 * Put a variable into the environment.
 * @param _env The environment.
 * @param _name The name of the variable.
 * @param _value The value of the variable.
 */
DLLEXPORT void env_put(env_t* _env, char* _name, object_t* _value);

/*
 * Get the value of a variable.
 * @param _env The environment.
 * @param _name The name of the variable.
 * @return The value of the variable.
 */
DLLEXPORT object_t* env_get(env_t* _env, char* _name);

/*
 * Get the parent environment.
 * @param _env The environment.
 * @return The parent environment.
 */
DLLEXPORT env_t* env_parent(env_t* _env);

/*
 * Free the environment.
 * @param _env The environment.
 */
DLLEXPORT void env_free(env_t* _env);

#endif