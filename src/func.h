#include "api/core/global.h"

#ifndef FUNC_H
#define FUNC_H

typedef struct func_ctx_struct {
    size_t codelen;
} func_ctx_t;

func_ctx_t* func_ctx_new();

#endif