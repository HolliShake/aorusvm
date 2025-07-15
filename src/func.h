#include "api/core/global.h"

#ifndef FUNC_H
#define FUNC_H

typedef struct func_ctx_struct {
    bool   global;
    size_t codelen;
} func_ctx_t;

/*
 * @brief Create a new function context
 * @return The new function context
 */
func_ctx_t* func_ctx_new();

/*
 * @brief End a block and add its length to the parent context
 * @param _ctx The context of the block to end
 * @param _parent The parent context
 */
void func_ctx_end_block(func_ctx_t* _ctx, func_ctx_t* _parent);

#endif