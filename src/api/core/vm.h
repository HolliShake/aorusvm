/**
 * @file vm.h
 * @brief The VM API (Do not modify this file).
 * @author Philipp Andrew Redondo
 * @date 2025-06-13
 * @version 0.1.0
 * @copyright MIT License
 * @note This file is part of the aorusvm project.
 */

#include "env.h"
#include "global.h"
#include "object.h"

#ifndef API_CORE_VM_H
#define API_CORE_VM_H

#define EVALUATION_STACK_SIZE 1024
#define SLICE_INT(value) (value & 0xFF), ((value >> 8) & 0xFF), ((value >> 16) & 0xFF), ((value >> 24) & 0xFF)
#define SLICE_LONG(value) (value & 0xFF), ((value >> 8) & 0xFF), ((value >> 16) & 0xFF), ((value >> 24) & 0xFF), ((value >> 32) & 0xFF), ((value >> 40) & 0xFF), ((value >> 48) & 0xFF), ((value >> 56) & 0xFF)
#define MAGIC_NUMBER 0xbadc0de
#define MAGIC_NUMBER_SLICE SLICE_INT(MAGIC_NUMBER) // Little-endian

/*
 * VM.
*/
typedef struct vm_struct vm_t;

/*
 * Name resolver function.
 * @param _vm The VM.
 * @param _name The name.
 */
typedef void (*vm_name_resolver_t)(vm_t*, env_t*, char*);

/*
 * Native function.
 * @param _vm The VM.
 */
typedef void (*vm_native_function)(vm_t*);

/*
 * Initialize the VM.
 */
DLLEXPORT void vm_init();

/*
 * Set the variable resolver.
 * @param _vm The VM.
 * @param _resolver The resolver.
 */
DLLEXPORT void vm_set_name_resolver(vm_t* _vm, vm_name_resolver_t _resolver);

/*
 * Name resolver.
 * @param _vm The VM.
 * @param _env The environment.
 * @param _name The name.
 */
DLLEXPORT void vm_name_resolver(vm_t* _vm, env_t* _env, char* _name);

/*
 * Run the main function.
 * @param _bytecode The bytecode.
 * @param _bytecode_size The size of the bytecode.
 */
DLLEXPORT void vm_run_main(uint8_t* _bytecode);

#endif
