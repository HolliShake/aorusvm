#include "api/core/global.h"

#ifndef CODE_H
#define CODE_H

typedef struct code_struct {
    /**** function **/
    size_t   param_count;
    /**** generic  **/
    size_t   size;
    uint8_t* bytecode;
} code_t;

code_t* code_new_function(size_t _param_count, uint8_t* _bytecode, size_t _size);
code_t* code_new_module(uint8_t* _bytecode, size_t _size);
void code_free(code_t* _code);

#endif