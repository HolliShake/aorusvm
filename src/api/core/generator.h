/**
 * @file generator.h
 * @brief The generator API (Do not modify this file).
 * @author Philipp Andrew Redondo
 * @date 2025-06-13
 * @version 0.1.0
 * @copyright MIT License
 * @note This file is part of the aorusvm project.
 */

#include "../ast/node.h"
#include "global.h"

#ifndef API_CORE_GENERATOR_H
#define API_CORE_GENERATOR_H

/*
 * @brief The generator struct.
 */
typedef struct generator_struct generator_t;

/*
 * @brief Creates a new generator.
 * 
 * @param _path The path to the file to generate.
 * @return The new generator.
 */
DLLEXPORT generator_t* generator_new(char* _fpath, char* _fdata);

/*
 * @brief Generates the bytecode for the program.
 * 
 * @param _generator The generator to use.
 * @param _program The program to generate the bytecode for.
 * @return The bytecode.
 */
DLLEXPORT uint8_t* generator_generate(generator_t* _generator, ast_node_t* _program);

/*
 * @brief Frees the generator.
 * 
 * @param _generator The generator to free.
 */
DLLEXPORT void generator_free(generator_t* _generator);


#endif