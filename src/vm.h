#include "api/core/vm.h"
#include "array.h"
#include "async.h"
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
    // For control flow statements
    VmBlockSignalCon,
    VmBlockSignalBrk,
} vm_block_signal_t;

typedef struct vm_struct {
    // evaluation stack
    object_t** evaluation_stack;
    async_t** queque;
    size_t sp;
    size_t aq;
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
    // Accumolator
    int acc;
} vm_t;

#endif
