#include "env.h"
#include "object.h"
#include "type.h"
#include "vm.h"

#ifndef GC_H
#define GC_H

#include "vm.h"

/*
 * Collect all the garbage.
 *
 * @param _vm The VM.
 */
void gc_collect_all(vm_t* _vm);

/*
 * Collect the garbage.
 *
 * @param _vm The VM.
 * @param _env The environment.
 */
void gc_collect(vm_t* _vm, env_t* _env);

#endif