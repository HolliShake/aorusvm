#include "api/ast/position.h"
#include "api/core/global.h"
#include "error.h"

#ifndef SCOPE_H
#define SCOPE_H

typedef enum scope_type_enum {
    ScopeTypeGlobal,
    ScopeTypeLocal,
    ScopeTypeClass,
    ScopeTypeFunction,
    ScopeTypeAsyncFunction,
    ScopeTypeCatch,
    ScopeTypeSingle,
    ScopeTypeLoop,
    ScopeTypeArray,
    ScopeTypeObject,
} scope_type_t;

typedef struct scope_value_struct {
    char*       name;
    bool        is_const;
    bool        is_global;
    position_t* position;
} scope_value_t;

typedef struct scope_node_struct scope_node_t;
typedef struct scope_node_struct {
    char*          name;
    scope_value_t value;
    scope_node_t*  next;
} scope_node_t;

typedef struct scope_struct scope_t;
typedef struct scope_struct {
    scope_t*       parent;
    scope_type_t   type;
    scope_node_t** buckets;
    size_t         bucket_count;
    size_t         size;
    // Captures
    size_t         capture_count;
    char**         captures;
    // Carry flags
    bool           is_returned;
    // Continues
    size_t         ccount;
    size_t         bcount;
    int*           con_jump;
    int*           brk_jump;
    // Block
    bool           is_block;
} scope_t;

/*
 * Create a new scope.
 *
 * @param _parent The parent scope.
 * @param _type The type of the scope.
 * @return The new scope.
 */
scope_t* scope_new(scope_t* _parent, scope_type_t _type);

/*
 * Create a new block scope.
 *
 * @param _parent The parent scope.
 * @param _type The type of the scope.
 * @return The new scope.
 */
scope_t* scope_block_new(scope_t* _parent, scope_type_t _type);

/*
 * Check if a scope has a value.
 *
 * @param _scope The scope to check.
 * @param _name The name of the value.
 * @param _recurse Whether to recurse up the scope chain.
 * @return True if the scope has the value, false otherwise.
 */
bool scope_has(scope_t* _scope, char* _name, bool _recurse);

/*
 * Check if a scope has a value in a function.
 *
 * @param _scope The scope to check.
 * @param _name The name of the value.
 * @return True if the scope has the value, false otherwise.
 */
bool scope_function_has(scope_t* _scope, char* _name);

/*
 * Put a value into a scope.
 *
 * @param _scope The scope to put the value into.
 * @param _name The name of the value.
 * @param _value The value to put into the scope.
 */
void scope_put(scope_t* _scope, char* _name, scope_value_t _value);

/*
 * Get a value from a scope.
 *
 * @param _scope The scope to get the value from.
 * @param _name The name of the value.
 * @param _recurse Whether to recurse up the scope chain.
 * @return The value.
 */
scope_value_t scope_get(scope_t* _scope, char* _name, bool _recurse);

/*
 * Check if a scope is global.
 *
 * @param _scope The scope to check.
 * @return True if the scope is global, false otherwise.
 */
bool scope_is_global(scope_t* _scope);

/*
 * Check if a scope is a local.
 *
 * @param _scope The scope to check.
 * @return True if the scope is a local, false otherwise.
 */
bool scope_is_local(scope_t* _scope);

/*
 * Check if a scope is a block.
 *
 * @param _scope The scope to check.
 * @return True if the scope is a block, false otherwise.
 */
bool scope_is_block(scope_t* _scope);

/*
 * Check if a scope is a class.
 *
 * @param _scope The scope to check.
 * @return True if the scope is a class, false otherwise.
 */
bool scope_is_class(scope_t* _scope);


/*
 * Check if a scope is a function.
 *
 * @param _scope The scope to check.
 * @return True if the scope is a function, false otherwise.
 */
bool scope_is_function(scope_t* _scope);

/*
 * Check if a scope is an async function.
 *
 * @param _scope The scope to check.
 * @return True if the scope is an async function, false otherwise.
 */
bool scope_is_async_function(scope_t* _scope);

/*
 * Check if a scope is a catch.
 *
 * @param _scope The scope to check.
 * @return True if the scope is a catch, false otherwise.
 */
bool scope_is_catch(scope_t* _scope);

/*
 * Check if a scope is a single.
 *
 * @param _scope The scope to check.
 * @return True if the scope is a single, false otherwise.
 */
bool scope_is_single(scope_t* _scope);
/*
 * Check if a scope is a loop.
 *
 * @param _scope The scope to check.
 * @return True if the scope is a loop, false otherwise.
 */
bool scope_is_loop(scope_t* _scope);

/*
 * Check if a scope is an array.
 *
 * @param _scope The scope to check.
 * @return True if the scope is an array, false otherwise.
 */
bool scope_is_array(scope_t* _scope);

/*
 * Check if a scope is an object.
 *
 * @param _scope The scope to check.
 * @param _recurse Whether to recurse up the scope chain.
 * @return True if the scope is an object, false otherwise.
 */
bool scope_is_object(scope_t* _scope, bool _recurse);

/*
 * Check if a scope is a capture.
 *
 * @param _scope The scope to check.
 * @return True if the scope is a capture, false otherwise.
 */
bool scope_save_capture(scope_t* _scope, char* _name);

/*
 * Free a scope.
 *
 * @param _scope The scope to free.
 */
void scope_free(scope_t* _scope);

#endif
