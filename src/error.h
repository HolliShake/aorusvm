#include "api/core/error.h"
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

#endif