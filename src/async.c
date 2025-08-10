#include "async.h"

async_t* async_new(size_t ip, size_t top, env_t* env, code_t* code, object_t* promise) {
    async_t* async = malloc(sizeof(async_t));
    if (!async) PD("failed to allocate memory for async_t");
    async->ip = ip;
    async->top = top;
    async->env = env;
    async->code = code;
    async->promise = promise;
    return async;
}

// 
void async_resolve(object_t* promise, object_t* value) {
    promise->value.i32 = (int) ASYNC_STATE_RESOLVED;
    promise->value.opaque = value;
}

void async_reject(object_t* promise, object_t* value) {
    promise->value.i32 = (int) ASYNC_STATE_REJECTED;
    promise->value.opaque = value;
}