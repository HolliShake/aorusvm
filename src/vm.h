#include "api/core/vm.h"
#include "array.h"
#include "code.h"
#include "decompiler.h"
#include "hashmap.h"
#include "iterator.h"
#include "object.h"
#include "range.h"

#ifndef VM_H
#define VM_H

typedef enum vm_block_signal_t {
    VmBlockSignalReturned,
    VmBlockSignalComplete,
    VmBlockSignalPending,
} vm_block_signal_t;

typedef struct vm_struct {
    // evaluation stack
    object_t** evaluation_stack;
    size_t sp;
    // function table
    size_t function_table_size;
    code_t** function_table_item;
    // function table counter
    size_t allocation_counter;
    // name resolver
    vm_name_resolver_t name_resolver;
    // root object
    object_t *root;
    object_t *tail;
    // singleton null
    object_t *null;
    // singleton boolean
    object_t *tobj;
    object_t *fobj;
    // env globals
    env_t* env;
} vm_t;

#endif