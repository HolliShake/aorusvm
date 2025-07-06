#include "object.h"

#ifndef ENV_H
#define ENV_H

typedef struct env_node_struct env_node_t;
typedef struct env_node_struct {
    char* name;
    object_t* value;
    env_node_t* next;
} env_node_t;

typedef struct env_struct {
    env_t* parent;
    env_node_t** buckets;
    size_t bucket_count;
    size_t size;
} env_t;


object_t** env_get_object_list(env_t* _env);

#endif