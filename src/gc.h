#include "env.h"
#include "object.h"
#include "vm.h"

#ifndef GC_H
#define GC_H

#include "vm.h"

void gc_collect(vm_t* _vm, env_t* _env);

#endif