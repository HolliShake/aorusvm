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

#define VERSION 0x001 // 0.0.1
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
typedef void (*vm_name_resolver_t)(env_t*, char*);

/*
 * Initialize the VM.
 */
DLLEXPORT void vm_init();

/*
 * Set the variable resolver.
 * @param _resolver The resolver.
 */
DLLEXPORT void vm_set_name_resolver(vm_name_resolver_t _resolver);

/*
 * Name resolver.
 * @param _env The environment.
 * @param _name The name.
 */
DLLEXPORT void vm_name_resolver(env_t* _env, char* _name);

/*
 * Links an object into the VM's heap memory and returns a handle to it.
 * The object will be tracked by the garbage collector.
 * @param _obj The object to link into the VM's heap
 * @return Handle to the linked object in heap memory
 */
DLLEXPORT object_t* vm_to_heap(object_t* _obj);

/*
 * Push an object to the evaluation stack.
 * @param _obj The object.
 */
DLLEXPORT void vm_push(object_t* _obj);

/*
 * Pop an object from the evaluation stack.
 * @return The object.
 */
DLLEXPORT object_t* vm_pop();

/*
 * Peek at the top of the evaluation stack.
 * @return The object.
 */
DLLEXPORT object_t* vm_peek();

/*
 * Load a null value into the evaluation stack.
 */
DLLEXPORT void vm_load_null();

/*
 * Load a boolean value into the evaluation stack.
 * @param _value The value.
 */
DLLEXPORT void vm_load_bool(bool _value);

/*
 * Define a global variable.
 * @param _name The name.
 * @param _value The value.
 */
DLLEXPORT void vm_define_global(char* _name, object_t* _value);

/*
 * Run the main function.
 * @param _bytecode The bytecode.
 * @param _bytecode_size The size of the bytecode.
 */
DLLEXPORT void vm_run_main(uint8_t* _bytecode);

#endif
