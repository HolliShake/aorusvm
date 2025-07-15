#include "api/core/env.h"
#include "api/core/global.h"

#ifndef CODE_H
#define CODE_H

typedef struct code_struct {
    bool     is_scoped;
    char*    file_name;
    char*    block_name;
    size_t   param_count;
    bool     is_async;
    size_t   size;
    uint8_t* bytecode;
    env_t*   environment;
} code_t;

/*
 * Create a new module code.
 *
 * @param _file_name The file name of the module.
 * @param _block_name The block name of the module.
 * @param _bytecode The bytecode of the module.
 * @param _size The size of the bytecode.
 * @return The new module code.
 */
code_t* code_new_module(
        char*    _file_name, 
        char*    _block_name);


/*
 * Create a new function code.
 *
 * @param _file_name The file name of the function.
 * @param _block_name The block name of the function.
 * @param _is_async Whether the function is async.
 * @param _param_count The number of parameters of the function.
 * @param _bytecode The bytecode of the function.
 * @param _size The size of the bytecode.
 * @return The new function code.
 */
code_t* code_new_function(
        char*    _file_name, 
        char*    _block_name,
        bool     _is_scoped,
        bool     _is_async, 
        size_t   _param_count,
        uint8_t* _bytecode, 
        size_t   _size);

/*
 * Create a new block code.
 *
 * @param _file_name The file name of the block.
 * @param _block_name The block name of the block.
 * @param _bytecode The bytecode of the block.
 * @param _size The size of the bytecode.
 * @return The new block code.
 */
code_t* code_new_block(
        char*    _file_name, 
        char*    _block_name,
        uint8_t* _bytecode, 
        size_t   _size);

/*
 * Free the code.
 *
 * @param _code The code to free.
 */
void code_free(code_t* _code);

#endif