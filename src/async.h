#include "api/core/global.h"
#include "code.h"
#include "object.h"
#include "env.h"

#ifndef ASYNC_H
#define ASYNC_H

typedef enum async_state_enum {
    ASYNC_STATE_PENDING,
    ASYNC_STATE_RESOLVED,
    ASYNC_STATE_REJECTED
} async_state_t;

typedef struct async_struct {
    size_t    ip;
    size_t    top;
    env_t*    env;
    code_t*   code;
    object_t* promise;
} async_t;

async_t* async_new(size_t ip, size_t top, env_t* env, code_t* code, object_t* promise);


// For promise
void async_resolve(object_t* promise, object_t* value);
void async_reject(object_t* promise, object_t* value);

#endif