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

async_promise_t* async_promise_new(async_state_t _state, object_t* _value) {
    async_promise_t* promise = malloc(sizeof(async_promise_t));
    if (!promise) PD("failed to allocate memory for async_promise_t");
    promise->state = _state;
    promise->value = _value;
    return promise;
}

// 
void async_resolve(object_t* _promise, object_t* _value) {
    async_promise_t* promise = (async_promise_t*) _promise->value.opaque;
    promise->state = ASYNC_STATE_RESOLVED;
    promise->value = _value;
}

void async_reject(object_t* _promise, object_t* _value) {
    async_promise_t* promise = (async_promise_t*) _promise->value.opaque;
    promise->state = ASYNC_STATE_REJECTED;
    promise->value = _value;
}

void async_free(async_t* _async) {
    env_free(_async->env);
    free(_async);
}