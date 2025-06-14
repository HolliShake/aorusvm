#include "api/ast/position.h"
#include "api/core/global.h"
#include "error.h"

#ifndef SCOPE_H
#define SCOPE_H

typedef enum scope_type_enum {
    ScopeTypeGlobal,
    ScopeTypeLocal,
    ScopeTypeFunction,
    ScopeTypeSingle,
    ScopeTypeLoop
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
    // Carry flags
    bool           is_returned;
} scope_t;

scope_t* scope_new(scope_t* _parent, scope_type_t _type);
bool scope_has(scope_t* _scope, char* _name, bool _recurse);
void scope_put(scope_t* _scope, char* _name, scope_value_t _value);
scope_value_t scope_get(scope_t* _scope, char* _name, bool _recurse);
bool scope_is_global(scope_t* _scope);
bool scope_is_local(scope_t* _scope);
bool scope_is_function(scope_t* _scope);
bool scope_is_single(scope_t* _scope);
bool scope_is_loop(scope_t* _scope);
void scope_free(scope_t* _scope);

#endif
