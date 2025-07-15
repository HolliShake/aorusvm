#include "api/core/env.h"
#include "api/core/global.h"
#include "api/core/object.h"
#include "env.h"
#include "error.h"
#include "internal.h"

#define ENV_BUCKET_COUNT 16
#define LOAD_FACTOR_THRESHOLD 0.75

DLLEXPORT env_t* env_new(env_t* _parent) {
    env_t* env = malloc(sizeof(env_t));
    ASSERTNULL(env, "failed to allocate memory for env");
    env->parent = _parent;
    env->buckets = calloc(ENV_BUCKET_COUNT, sizeof(env_node_t*));
    for (size_t i = 0; i < ENV_BUCKET_COUNT; i++) env->buckets[i] = NULL;
    ASSERTNULL(env->buckets, "failed to allocate memory for buckets");
    env->bucket_count = ENV_BUCKET_COUNT;
    env->size = 0;
    env->closure = NULL;
    return env;
}

INTERNAL void env_rehash(env_t* _env) {
    size_t new_bucket_count = _env->bucket_count * 2;
    env_node_t** new_buckets = calloc(new_bucket_count, sizeof(env_node_t*));
    for (size_t i = 0; i < new_bucket_count; i++) new_buckets[i] = NULL;
    ASSERTNULL(new_buckets, "failed to allocate memory for buckets");

    for (size_t i = 0; i < _env->bucket_count; i++) {
        env_node_t* node = _env->buckets[i];
        while (node) {
            env_node_t* next = node->next;
            size_t index = hash64(node->name) % new_bucket_count;
            node->next = new_buckets[index];
            new_buckets[index] = node;
            node = next;
        }
    }

    free(_env->buckets);
    _env->buckets = new_buckets;
    _env->bucket_count = new_bucket_count;
}

DLLEXPORT bool env_has(env_t* _env, char* _name, bool _recurse) {
    if (_env == NULL) return false;
    
    env_t* current_env = _env;
    size_t hash = hash64(_name);
    
    do {
        size_t index = hash % current_env->bucket_count;
        env_node_t* node = current_env->buckets[index];
        
        while (node) {
            if (strcmp(node->name, _name) == 0) return true;
            node = node->next;
        }
        
        // Check closure environment if it exists
        if (current_env->closure != NULL && env_has(current_env->closure, _name, _recurse)) {
            return true;
        }
        
        current_env = _recurse ? current_env->parent : NULL;
    } while (current_env);
    
    return false;
}

DLLEXPORT void env_put(env_t* _env, char* _name, object_t* _value) {
    if (_env == NULL) return;
    size_t hash = hash64(_name);
    size_t index = hash % _env->bucket_count;
    env_node_t* node = _env->buckets[index];

    if (node != NULL) {
        env_node_t* current = node;
        while (current) {
            if (strcmp(current->name, _name) == 0) {
                current->value = _value;
                return;
            }
            if (current->next == NULL) break;
            current = current->next;
        }
        node = malloc(sizeof(env_node_t));
        ASSERTNULL(node, "failed to allocate memory for env node");
        node->name = string_allocate(_name);
        node->value = _value;
        node->next = NULL;
        current->next = node;

        _env->size++;  // <-- ADD THIS
    } else {
        node = malloc(sizeof(env_node_t));
        ASSERTNULL(node, "failed to allocate memory for env node");
        node->name = string_allocate(_name);
        node->value = _value;
        node->next = NULL;
        _env->buckets[index] = node;

        _env->size++;  // already here
    }

    if (_env->size > _env->bucket_count * LOAD_FACTOR_THRESHOLD) {
        env_rehash(_env);
    }
}


DLLEXPORT object_t* env_get(env_t* _env, char* _name) {
    if (_env == NULL) return NULL;
    size_t hash = hash64(_name);
    env_t* current_env = _env;
    
    while (current_env) {
        // Check in current environment
        size_t index = hash % current_env->bucket_count;
        env_node_t* node = current_env->buckets[index];
        while (node) {
            if (strcmp(node->name, _name) == 0) {
                return node->value;
            }
            node = node->next;
        }
        
        // Check in closure environment if it exists
        if (current_env->closure != NULL) {
            object_t* closure_value = env_get(current_env->closure, _name);
            if (closure_value != NULL) {
                return closure_value;
            }
        }
        
        // Move up to parent environment
        current_env = current_env->parent;
    }
    
    return NULL;
}

DLLEXPORT env_t* env_parent(env_t* _env) {
    return _env->parent;
}

DLLEXPORT void env_free(env_t* _env) {
    for (size_t i = 0; i < _env->bucket_count; i++) {
        env_node_t* node = _env->buckets[i];
        while (node) {
            env_node_t* next = node->next;
            free(node->name);
            free(node);
            node = next;
        }
    }
    free(_env->buckets);
    free(_env);
}

object_t** env_get_object_list(env_t* _env) {
    // First count the number of objects
    size_t count = 0;
    for (size_t i = 0; i < _env->bucket_count; i++) {
        env_node_t* node = _env->buckets[i];
        while (node) {
            count++;
            node = node->next;
        }
    }
    
    // Allocate memory for all objects plus NULL terminator
    object_t** list = malloc(sizeof(object_t*) * (count + 1));
    if (list == NULL) return NULL;
    
    // Fill the list
    size_t index = 0;
    for (size_t i = 0; i < _env->bucket_count; i++) {
        env_node_t* node = _env->buckets[i];
        while (node) {
            list[index++] = node->value;
            node = node->next;
        }
    }
    list[index] = NULL;
    
    return list;
}

void env_dump_symbols(env_t* _env) {
    printf("+-------------------------+\n");
    printf("|        ENV DUMP         |\n");
    printf("+-------------------------+\n");
    if (_env == NULL) return;
    if (_env->parent != NULL) {
        printf("| [PARENT]                |\n");
        for (size_t i = 0; i < _env->parent->bucket_count; i++) {
            env_node_t* node = _env->parent->buckets[i];
            while (node) {
                if (strlen(node->name) > 24) {
                    printf("| %.20s... |\n", node->name);
                } else {
                    printf("| %s", node->name);
                    for (size_t j = 0; j < 24 - strlen(node->name); j++) {
                        printf(" ");
                    }
                    printf("|\n");
                }
                node = node->next;
            }
        }
    }
    if (_env->closure != NULL) {
        printf("+-------------------------+\n");
        printf("| [CLOSURE]               |\n");
        for (size_t i = 0; i < _env->closure->bucket_count; i++) {
            env_node_t* node = _env->closure->buckets[i];
            while (node) {
                if (strlen(node->name) > 24) {
                    printf("| %.20s... |\n", node->name);
                } else {
                    printf("| %s", node->name);
                    for (size_t j = 0; j < 24 - strlen(node->name); j++) {
                        printf(" ");
                    }
                    printf("|\n");
                }
                node = node->next; 
            }
        }
    }
    printf("+-------------------------+\n");
    printf("| [LOCALS]                |\n");
    for (size_t i = 0; i < _env->bucket_count; i++) {
        env_node_t* node = _env->buckets[i];
        while (node) {
            if (strlen(node->name) > 24) {
                printf("| %.20s... |\n", node->name);
            } else {
                printf("| %s", node->name);
                for (size_t j = 0; j < 24 - strlen(node->name); j++) {
                    printf(" ");
                }
                printf("|\n");
            }
            node = node->next;
        }
    }
    printf("+-------------------------+\n");
}