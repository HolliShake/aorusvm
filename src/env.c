#include "api/core/env.h"
#include "api/core/global.h"
#include "api/core/object.h"
#include "error.h"
#include "internal.h"

#define ENV_BUCKET_COUNT 16
#define LOAD_FACTOR_THRESHOLD 0.75

typedef struct env_node_struct env_node_t;
typedef struct env_node_struct {
    char* name;
    object_t* value;
    env_node_t* next;
} env_node_t;

struct env_struct {
    env_t* parent;
    env_node_t** buckets;
    size_t bucket_count;
    size_t size;
};

env_t* env_new(env_t* _parent) {
    env_t* env = malloc(sizeof(env_t));
    ASSERTNULL(env, ERROR_ALLOCATING_ENV);
    env->parent = _parent;
    env->buckets = calloc(ENV_BUCKET_COUNT, sizeof(env_node_t*));
    ASSERTNULL(env->buckets, ERROR_ALLOCATING_BUCKETS);
    env->bucket_count = ENV_BUCKET_COUNT;
    env->size = 0;
    return env;
}

static void env_rehash(env_t* _env) {
    size_t new_bucket_count = _env->bucket_count * 2;
    env_node_t** new_buckets = calloc(new_bucket_count, sizeof(env_node_t*));
    ASSERTNULL(new_buckets, ERROR_ALLOCATING_BUCKETS);

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

bool env_has(env_t* _env, char* _name, bool _recurse) {
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

void env_put(env_t* _env, char* _name, object_t* _value) {
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
    ASSERTNULL(node, ERROR_ALLOCATING_ENV_NODE);
    node->name = _name;
    node->value = _value;
    node->next = _env->buckets[index];
    _env->buckets[index] = node;

    // Check load factor and rehash if needed
    if (++_env->size > _env->bucket_count * LOAD_FACTOR_THRESHOLD) {
        env_rehash(_env);
    }
}

object_t* env_get(env_t* _env, char* _name) {
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

void env_free(env_t* _env) {
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