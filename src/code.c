#include "code.h"

code_t* code_new_function(size_t _param_count, uint8_t* _bytecode, size_t _size) {
    code_t* code = malloc(sizeof(code_t));
    code->param_count = _param_count;
    code->size = _size;
    code->bytecode = _bytecode;
    return code;
}

code_t* code_new_module(uint8_t* _bytecode, size_t _size) {
    code_t* code = malloc(sizeof(code_t));
    code->param_count = 0;
    code->size = _size;
    code->bytecode = _bytecode;
    return code;
}

void code_free(code_t* _code) {
    free(_code->bytecode);
    free(_code);
}