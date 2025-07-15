#include "func.h"

func_ctx_t* func_ctx_new() {
    func_ctx_t* ctx = malloc(sizeof(func_ctx_t));
    if (!ctx) PD("failed to allocate memory for func_ctx_t");
    ctx->codelen = 0;
    ctx->global  = false;
    return ctx;
}


void func_ctx_end_block(func_ctx_t* _ctx, func_ctx_t* _parent) {
    _parent->codelen += _ctx->codelen;
}
