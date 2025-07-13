#include "internal.h"
#include "scope.h"

#define SCOPE_BUCKET_COUNT 16
#define LOAD_FACTOR_THRESHOLD 0.75

scope_t* scope_new(scope_t* _parent, scope_type_t _type) {
    scope_t* scope = (scope_t*) malloc(sizeof(scope_t));
    ASSERTNULL(scope, "failed to allocate memory for scope");
    scope->type = _type;
    scope->parent = _parent;
    scope->buckets = (scope_node_t**) calloc(SCOPE_BUCKET_COUNT, sizeof(scope_node_t*));
    ASSERTNULL(scope->buckets, "failed to allocate memory for buckets");
    scope->bucket_count = SCOPE_BUCKET_COUNT;
    scope->size = 0;
    scope->capture_count = 0;
    scope->captures = (char**) malloc(sizeof(char*));
    scope->captures[0] = NULL;
    return scope;
}

INTERNAL void scope_rehash(scope_t* _scope) {
    size_t new_bucket_count = _scope->bucket_count * 2;
    scope_node_t** new_buckets = (scope_node_t**) calloc(new_bucket_count, sizeof(scope_node_t*));
    ASSERTNULL(new_buckets, "failed to allocate memory for buckets");

    // Rehash existing entries
    for (size_t i = 0; i < _scope->bucket_count; i++) {
        scope_node_t* node = _scope->buckets[i];
        while (node) {
            scope_node_t* next = node->next;
            size_t index = hash64(node->name) % new_bucket_count;
            node->next = new_buckets[index];
            new_buckets[index] = node;
            node = next;
        }
    }

    free(_scope->buckets);
    _scope->buckets = new_buckets;
    _scope->bucket_count = new_bucket_count;
}

bool scope_has(scope_t* _scope, char* _name, bool _recurse) {
    size_t hash = hash64(_name);
    do {
        scope_node_t* node = _scope->buckets[hash % _scope->bucket_count];
        while (node) {
            if (strcmp(node->name, _name) == 0) return true;
            node = node->next;
        }
        _scope = _recurse ? _scope->parent : NULL;
    } while (_scope);
    return false;
}

bool scope_function_has(scope_t* _scope, char* _name) {
    scope_t* current = _scope;
    while (current != NULL) {
        if (current->type == ScopeTypeLocal && current->parent != NULL && current->parent->type == ScopeTypeFunction) {
            if (scope_has(current, _name, false)) return true;
            return false;
        }
        current = current->parent;
    }
    return false;
}

void scope_put(scope_t* _scope, char* _name, scope_value_t _value) {
    size_t hash = hash64(_name);
    size_t index = hash % _scope->bucket_count;
    scope_node_t* node = _scope->buckets[index];

    // Check for existing entry
    while (node) {
        if (strcmp(node->name, _name) == 0) {
            node->value = _value;
            return;
        }
        node = node->next;
    }

    // Create new node and insert at head of chain
    node = (scope_node_t*) malloc(sizeof(scope_node_t));
    ASSERTNULL(node, "failed to allocate memory for scope node");
    node->name = _name;
    node->value = _value;
    node->next = _scope->buckets[index];
    _scope->buckets[index] = node;

    // Check load factor and rehash if needed
    if (++_scope->size > _scope->bucket_count * LOAD_FACTOR_THRESHOLD) {
        scope_rehash(_scope);
    }
}

scope_value_t scope_get(scope_t* _scope, char* _name, bool _recurse) {
    size_t hash = hash64(_name);
    do {
        scope_node_t* node = _scope->buckets[hash % _scope->bucket_count];
        while (node) {
            if (strcmp(node->name, _name) == 0) return node->value;
            node = node->next;
        }
        _scope = _recurse ? _scope->parent : NULL;
    } while (_scope);
    PD("variable %s not found", _name);
}

bool scope_is_global(scope_t* _scope) {
    return _scope->type == ScopeTypeGlobal;
}

bool scope_is_local(scope_t* _scope) {
    return _scope->type == ScopeTypeLocal;
}

bool scope_is_function(scope_t* _scope) {
    scope_t* current = _scope;
    while (current != NULL) {
        if (current->type == ScopeTypeFunction) return true;
        current = current->parent;
    }
    return false;
}

bool scope_is_catch(scope_t* _scope) {
    scope_t* current = _scope;
    while (current != NULL) {
        if (current->type == ScopeTypeCatch) return true;
        current = current->parent;
    }
    return false;
}

bool scope_is_single(scope_t* _scope) {
    return _scope->type == ScopeTypeSingle;
}

bool scope_is_loop(scope_t* _scope) {
    scope_t* current = _scope;
    while (current != NULL) {
        if (current->type == ScopeTypeLoop) return true;
        current = current->parent;
    }
    return false;
}

bool scope_is_array(scope_t* _scope) {
    return _scope->type == ScopeTypeArray;
}

bool scope_is_object(scope_t* _scope) {
    return _scope->type == ScopeTypeObject;
}

bool scope_save_capture(scope_t* _scope, char* _name) {
    // Save only the captures into a function scope
    scope_t* current = _scope;
    while (current != NULL) {
        bool exists = false;
        for (size_t i = 0; i < current->capture_count; i++) {
            if (strcmp(current->captures[i], _name) == 0) {
                exists = true;
                break;
            }
        }
        if (exists) return true;
        if (current->type == ScopeTypeFunction) {
            current->captures[current->capture_count++] = strdup(_name);
            current->captures = (char**) realloc(current->captures, sizeof(char*) * (current->capture_count + 1));
            current->captures[current->capture_count] = NULL;
            return true;
        }
        current = current->parent;
    }
    return false;
}

void scope_free(scope_t* _scope) {
    for (size_t i = 0; i < _scope->bucket_count; i++) {
        scope_node_t* node = _scope->buckets[i];
        while (node) {
            scope_node_t* next = node->next;
            free(node);
            node = next;
        }
    }
    for (size_t i = 0; i < _scope->capture_count; i++) {
        free(_scope->captures[i]);
    }
    free(_scope->captures);
    free(_scope->buckets);
    free(_scope);
}