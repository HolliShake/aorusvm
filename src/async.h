#include "api/core/async.h"
#include "api/core/global.h"
#include "code.h"
#include "env.h"
#include "object.h"

#ifndef ASYNC_H
#define ASYNC_H

typedef struct async_struct {
    size_t    ip;
    size_t    top;
    env_t*    env;
    code_t*   code;
    object_t* promise;
} async_t;

typedef struct async_promise_struct {
    async_state_t state;
    object_t*     value;
} async_promise_t;

// Create a new async
async_t* async_new(size_t ip, size_t top, env_t* env, code_t* code, object_t* promise);

// Create a new async promise
async_promise_t* async_promise_new(async_state_t _state, object_t* _value);

// For promise
// Resolve promise
void async_resolve(object_t* _promise, object_t* _value);

// Reject promise
void async_reject(object_t* _promise, object_t* _value);

void async_free(async_t* _async);

#endif