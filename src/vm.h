#include "api/core/vm.h"

#ifndef VM_H
#define VM_H

typedef struct vm_struct {
    // evaluation stack
    object_t **evaluation_stack;
    size_t sp;
    // counter
    size_t allocation_counter;
    // name resolver
    vm_name_resolver_t name_resolver;
    // root object
    object_t *root;
    // singleton null
    object_t *null;
    // singleton boolean
    object_t *tobj;
    object_t *fobj;
} vm_t;

#endif