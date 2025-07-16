#include "code.h"

code_t* code_new_module(char* _file_name, char* _block_name) {
    code_t* code = malloc(sizeof(code_t));
    ASSERTNULL(code, "Failed to allocate memory for code");
    code->is_scoped   = false;
    code->file_name   = _file_name;
    code->block_name  = _block_name;
    code->is_async    = false;
    code->param_count = 0;
    code->size        = 0;
    code->bytecode    = (uint8_t*) malloc(sizeof(uint8_t));
    code->environment = env_new(NULL);
    return code;
}


code_t* code_new_function(char* _file_name, char* _block_name, bool _is_scoped, bool _is_async, size_t _param_count, uint8_t* _bytecode, size_t _size) {
    code_t* code = malloc(sizeof(code_t));
    ASSERTNULL(code, "Failed to allocate memory for code");
    code->is_scoped   = _is_scoped;
    code->file_name   = _file_name;
    code->block_name  = _block_name;
    code->is_async    = _is_async;
    code->param_count = _param_count;
    code->size        = _size;
    code->bytecode    = _bytecode;
    code->environment = env_new(NULL);
    return code;
}

code_t* code_new_block(char* _file_name, char* _block_name, uint8_t* _bytecode, size_t _size) {
    code_t* code = malloc(sizeof(code_t));
    ASSERTNULL(code, "Failed to allocate memory for code");
    code->is_scoped   = true;
    code->file_name   = _file_name;
    code->block_name  = _block_name;
    code->is_async    = false;
    code->param_count = 0;
    code->size        = _size;
    code->bytecode    = _bytecode;
    code->environment = env_new(NULL);
    return code;
}

void code_free(code_t* _code) {
    free(_code->file_name);
    free(_code->block_name);
    free(_code->bytecode);
    free(_code);
}