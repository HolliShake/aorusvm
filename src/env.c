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
    ASSERTNULL(env->buckets, "failed to allocate memory for buckets");
    env->bucket_count = ENV_BUCKET_COUNT;
    env->size = 0;
    env->ref_count = 0;
    return env;
}

INTERNAL void env_rehash(env_t* _env) {
    size_t new_bucket_count = _env->bucket_count * 2;
    env_node_t** new_buckets = calloc(new_bucket_count, sizeof(env_node_t*));
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
    size_t hash = hash64(_name);
    do {
        env_node_t* node = _env->buckets[hash % _env->bucket_count];
        while (node) {
            if (strcmp(node->name, _name) == 0) return true;
            node = node->next;
        }
        _env = _recurse ? _env->parent : NULL;
    } while (_env);
    return false;
}

DLLEXPORT void env_put(env_t* _env, char* _name, object_t* _value) {
    size_t hash = hash64(_name);
    size_t index = hash % _env->bucket_count;
    env_node_t* node = _env->buckets[index];

    // Check for existing entry
    while (node) {
        if (strcmp(node->name, _name) == 0) {
            node->value = _value;
            return;
        }
        node = node->next;
    }

    // Create new node and insert at head of chain
    node = malloc(sizeof(env_node_t));
    ASSERTNULL(node, "failed to allocate memory for env node");
    node->name = _name;
    node->value = _value;
    node->next = _env->buckets[index];
    _env->buckets[index] = node;

    // Check load factor and rehash if needed
    if (++_env->size > _env->bucket_count * LOAD_FACTOR_THRESHOLD) {
        env_rehash(_env);
    }
}

DLLEXPORT object_t* env_get(env_t* _env, char* _name) {
    size_t hash = hash64(_name);
    while (_env) {
        env_node_t* node = _env->buckets[hash % _env->bucket_count];
        while (node) {
            if (strcmp(node->name, _name) == 0) {
                return node->value;
            }
            node = node->next;
        }
        _env = _env->parent;
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
}

object_t** env_get_object_list(env_t* _env) {
    size_t index = 0;
    object_t** list = malloc(sizeof(object_t*));
    list[0] = NULL;

    for (size_t i = 0; i < _env->bucket_count; i++) {
        env_node_t* node = _env->buckets[i];
        while (node) {
            list[index++] = node->value;
            list = (object_t**) realloc(list, sizeof(object_t*) * (index + 1));
            list[index] = NULL;
            node = node->next;
        }
    }

    return list;
}

void env_inc_ref(env_t* _env) {
    _env->ref_count++;
}

void env_dec_ref(env_t* _env) {
    _env->ref_count--;
}