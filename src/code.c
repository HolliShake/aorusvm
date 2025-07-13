#include "code.h"

code_t* code_new_function(bool _is_async, size_t _param_count, uint8_t* _bytecode, size_t _size) {
    code_t* code = malloc(sizeof(code_t));
    code->is_async = _is_async;
    code->param_count = _param_count;
    code->size = _size;
    code->bytecode = _bytecode;
    code->environment = NULL;
    return code;
}

code_t* code_new_module(uint8_t* _bytecode, size_t _size) {
    code_t* code = malloc(sizeof(code_t));
    code->param_count = 0;
    code->size = _size;
    code->bytecode = _bytecode;
    code->environment = NULL;
    return code;
}

void code_free(code_t* _code) {
    free(_code->bytecode);
    free(_code);
}