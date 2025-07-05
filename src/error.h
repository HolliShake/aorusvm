#include "api/core/global.h"
#include "internal.h"
#include "position.h"

#ifndef ERROR_H
#define ERROR_H

#define ERROR_ALLOCATING_AST_NODE "failed to allocate memory for ast node"
#define ERROR_ALLOCATING_AST_NODE_LIST "failed to allocate memory for ast node list"
#define ERROR_ALLOCATING_BUCKETS "failed to allocate memory for buckets"
#define ERROR_ALLOCATING_BYTECODE "failed to allocate memory for bytecode"
#define ERROR_ALLOCATING_ENV "failed to allocate memory for env"
#define ERROR_ALLOCATING_ENV_NODE "failed to allocate memory for env node"
#define ERROR_ALLOCATING_EVALUATION_STACK "failed to allocate memory for evaluation stack"
#define ERROR_ALLOCATING_GENERATOR "failed to allocate memory for generator"
#define ERROR_ALLOCATING_OBJECT "failed to allocate memory for object"
#define ERROR_ALLOCATING_POSITION "failed to allocate memory for position"
#define ERROR_ALLOCATING_SCOPE "failed to allocate memory for scope"
#define ERROR_ALLOCATING_SCOPE_NODE "failed to allocate memory for scope node"
#define ERROR_ALLOCATING_STRING "failed to allocate memory for string"
#define ERROR_ALLOCATING_VM "failed to allocate memory for vm"
#define ERROR_VM_NOT_INITIALIZED "VM is not initialized"

#define __THROW_ERROR(fpath, fdata, position, message, ...) \
    throw_error(__FILE__, __func__, __LINE__, fpath, fdata, string_format(message, ##__VA_ARGS__), position);

void throw_error(const char* _caller_file, const char* _caller_func, int _caller_line, char* _fpath, char* _fdata, char* _message, position_t* _position);

#endif