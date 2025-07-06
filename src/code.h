#include "api/core/global.h"

#ifndef CODE_H
#define CODE_H

typedef struct code_struct {
    /**** function **/
    size_t   param_count;
    bool     is_async;
    /**** generic  **/
    size_t   size;
    uint8_t* bytecode;
} code_t;

/*
 * Create a new function code.
 *
 * @param _is_async Whether the function is async.
 * @param _param_count The number of parameters of the function.
 * @param _bytecode The bytecode of the function.
 * @param _size The size of the bytecode.
 * @return The new function code.
 */
code_t* code_new_function(bool _is_async, size_t _param_count, uint8_t* _bytecode, size_t _size);

/*
 * Create a new module code.
 *
 * @param _bytecode The bytecode of the module.
 * @param _size The size of the bytecode.
 * @return The new module code.
 */
code_t* code_new_module(uint8_t* _bytecode, size_t _size);

/*
 * Free the code.
 *
 * @param _code The code to free.
 */
void code_free(code_t* _code);

#endif