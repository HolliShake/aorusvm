#include "api/ast/node.h"
#include "api/ast/position.h"
#include "api/core/global.h"

#ifndef NODE_H
#define NODE_H

typedef enum ast_node_type_struct {
    // Expressions
    AstName,
    AstInt,
    AstLong,
    AstFloat,
    AstDouble,
    AstBoolean,
    AstString,
    AstNull,
    AstCall,
    // Unary operators
    AstUnaryPlus,
    // Binary operators
    AstBinaryMul,
    AstBinaryDiv,
    AstBinaryMod,
    AstBinaryAdd,
    AstBinarySub,
    AstBinaryShl,
    AstBinaryShr,
    AstCmpLt,
    AstCmpLte,
    AstCmpGt,
    AstCmpGte,
    AstCmpEq,
    AstCmpNe,
    AstBinaryAnd,
    AstBinaryOr,
    AstBinaryXor,
    AstLogicalAnd,
    AstLogicalOr,
    // Statements and declarations
    AstVarStatement,
    AstConstStatement,
    AstLocalStatement,
    AstIfStatement,
    AstWhileStatement,
    AstDoWhileStatement,
    AstReturnStatement,
    AstExpressionStatement,
    AstBlockStatement,
    AstFunctionNode,
    AstAsyncFunctionNode,
    AstProgram,
} ast_node_type_t;

typedef struct ast_node_struct {
    ast_node_type_t type;
    union value_union {
        int    i32;
        long   i64;
        float  f32;
        double f64;
    } value;
    char*           str0;
    ast_node_t*     ast0;
    ast_node_t*     ast1;
    ast_node_t*     ast2;
    ast_node_t*     ast3;
    ast_node_list_t array0;
    ast_node_list_t array1;
    ast_node_list_t array2;
    position_t*     position;
} ast_node_t;

#endif