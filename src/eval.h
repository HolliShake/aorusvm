#include "api/core/global.h"
#include "api/ast/node.h"
#include "internal.h"
#include "node.h"

#ifndef EVAL_H
#define EVAL_H

typedef enum eval_result_type_enum {
    EvalInt,
    EvalLong,
    EvalFloat,
    EvalDouble,
    EvalString,
    EvalBoolean,
    EvalNull,
    EvalError,
} eval_result_type_t;

typedef struct eval_result_struct {
    eval_result_type_t type;
    union eval_result_union {
        int    i32;
        long   i64;
        float  f32;
        double f64;
        void*  ptr;
    } value;
} eval_result_t;

eval_result_t eval_eval(ast_node_t* _expression);

#endif