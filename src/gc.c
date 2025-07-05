#include "gc.h"


INTERNAL void gc_mark_vm_content(vm_t* _vm) {
    _vm->tobj->marked = true;
    _vm->fobj->marked = true;
    _vm->null->marked = true;

    // mark the evaluation stack
    object_t* current = _vm->root;
}

void gc_collect(vm_t* _vm) {
    // reset the allocation counter
    _vm->allocation_counter = 0;
}