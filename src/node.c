#include "api/ast/node.h"
#include "api/ast/position.h"
#include "api/core/global.h"
#include "error.h"
#include "node.h"

INTERNAL
ast_node_t* ast_node_new(ast_node_type_t type, position_t* position) {
    ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
    ASSERTNULL(node, ERROR_ALLOCATING_AST_NODE);
    node->type = type;
    node->position = position;
    node->str0   = NULL;
    node->ast0   = NULL;
    node->ast1   = NULL;
    node->ast2   = NULL;
    node->ast3   = NULL;
    node->array0 = NULL;
    node->array1 = NULL;
    node->array2 = NULL;
    return node;
}

DLLEXPORT ast_node_t* ast_name_node(position_t* _position, char* _value) {
    ast_node_t* node = ast_node_new(AstName, _position);
    node->str0 = _value;
    return node;
}

DLLEXPORT ast_node_t* ast_int_node(position_t* _position, int _value) {
    ast_node_t* node = ast_node_new(AstInt, _position);
    node->value.i32 = _value;
    return node;
}

DLLEXPORT ast_node_t* ast_long_node(position_t* _position, long _value) {
    ast_node_t* node = ast_node_new(AstLong, _position);
    node->value.i64 = _value;
    return node;
}

DLLEXPORT ast_node_t* ast_float_node(position_t* _position, float _value) {
    ast_node_t* node = ast_node_new(AstFloat, _position);
    node->value.f32 = _value;
    return node;
}

DLLEXPORT ast_node_t* ast_double_node(position_t* _position, double _value) {
    ast_node_t* node = ast_node_new(AstDouble, _position);
    node->value.f64 = _value;
    return node;
}

DLLEXPORT ast_node_t* ast_boolean_node(position_t* _position, bool _value) {
    ast_node_t* node = ast_node_new(AstBoolean, _position);
    node->value.i32 = _value;
    return node;
}

DLLEXPORT ast_node_t* ast_string_node(position_t* _position, char* _value) {
    ast_node_t* node = ast_node_new(AstString, _position);
    node->str0 = _value;
    return node;
}

DLLEXPORT ast_node_t* ast_null_node(position_t* _position) {
    ast_node_t* node = ast_node_new(AstNull, _position);
    return node;
}

DLLEXPORT ast_node_t* ast_binary_mul_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs) {
    ast_node_t* node = ast_node_new(AstBinaryMul, _position);
    node->ast0 = _lhs;
    node->ast1 = _rhs;
    return node;
}

DLLEXPORT ast_node_t* ast_binary_div_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs) {
    ast_node_t* node = ast_node_new(AstBinaryDiv, _position);
    node->ast0 = _lhs;
    node->ast1 = _rhs;
    return node;
}

DLLEXPORT ast_node_t* ast_binary_mod_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs) {
    ast_node_t* node = ast_node_new(AstBinaryMod, _position);
    node->ast0 = _lhs;
    node->ast1 = _rhs;
    return node;
}

DLLEXPORT ast_node_t* ast_binary_add_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs) {
    ast_node_t* node = ast_node_new(AstBinaryAdd, _position);
    node->ast0 = _lhs;
    node->ast1 = _rhs;
    return node;
}

DLLEXPORT ast_node_t* ast_binary_sub_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs) {
    ast_node_t* node = ast_node_new(AstBinarySub, _position);
    node->ast0 = _lhs;
    node->ast1 = _rhs;
    return node;
}

DLLEXPORT ast_node_t* ast_binary_shl_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs) {
    ast_node_t* node = ast_node_new(AstBinaryShl, _position);
    node->ast0 = _lhs;
    node->ast1 = _rhs;
    return node;
}

DLLEXPORT ast_node_t* ast_binary_shr_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs) {
    ast_node_t* node = ast_node_new(AstBinaryShr, _position);
    node->ast0 = _lhs;
    node->ast1 = _rhs;
    return node;
}

DLLEXPORT ast_node_t* ast_cmp_lt_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs) {
    ast_node_t* node = ast_node_new(AstCmpLt, _position);
    node->ast0 = _lhs;
    node->ast1 = _rhs;
    return node;
}

DLLEXPORT ast_node_t* ast_cmp_lte_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs) {
    ast_node_t* node = ast_node_new(AstCmpLte, _position);
    node->ast0 = _lhs;
    node->ast1 = _rhs;
    return node;
}

DLLEXPORT ast_node_t* ast_cmp_gt_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs) {
    ast_node_t* node = ast_node_new(AstCmpGt, _position);
    node->ast0 = _lhs;
    node->ast1 = _rhs;
    return node;
}

DLLEXPORT ast_node_t* ast_cmp_gte_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs) {
    ast_node_t* node = ast_node_new(AstCmpGte, _position);
    node->ast0 = _lhs;
    node->ast1 = _rhs;
    return node;
}

DLLEXPORT ast_node_t* ast_cmp_eq_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs) {
    ast_node_t* node = ast_node_new(AstCmpEq, _position);
    node->ast0 = _lhs;
    node->ast1 = _rhs;
    return node;
}

DLLEXPORT ast_node_t* ast_cmp_ne_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs) {
    ast_node_t* node = ast_node_new(AstCmpNe, _position);
    node->ast0 = _lhs;
    node->ast1 = _rhs;
    return node;
}

DLLEXPORT ast_node_t* ast_binary_and_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs) {
    ast_node_t* node = ast_node_new(AstBinaryAnd, _position);
    node->ast0 = _lhs;
    node->ast1 = _rhs;
    return node;
}

DLLEXPORT ast_node_t* ast_binary_or_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs) {
    ast_node_t* node = ast_node_new(AstBinaryOr, _position);
    node->ast0 = _lhs;
    node->ast1 = _rhs;
    return node;
}

DLLEXPORT ast_node_t* ast_binary_xor_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs) {
    ast_node_t* node = ast_node_new(AstBinaryXor, _position);
    node->ast0 = _lhs;
    node->ast1 = _rhs;
    return node;
}

DLLEXPORT ast_node_t* ast_logical_and_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs) {
    ast_node_t* node = ast_node_new(AstLogicalAnd, _position);
    node->ast0 = _lhs;
    node->ast1 = _rhs;
    return node;
}

DLLEXPORT ast_node_t* ast_logical_or_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs) {
    ast_node_t* node = ast_node_new(AstLogicalOr, _position);
    node->ast0 = _lhs;
    node->ast1 = _rhs;
    return node;
}

DLLEXPORT ast_node_t* ast_var_statement_node(position_t* _position, ast_node_list_t _names, ast_node_list_t _values) {
    ast_node_t* node = ast_node_new(AstVarStatement, _position);
    node->array0 = _names;
    node->array1 = _values;
    return node;
}

DLLEXPORT ast_node_t* ast_const_statement_node(position_t* _position, ast_node_list_t _names, ast_node_list_t _values) {
    ast_node_t* node = ast_node_new(AstConstStatement, _position);
    node->array0 = _names;
    node->array1 = _values;
    return node;
}

DLLEXPORT ast_node_t* ast_local_statement_node(position_t* _position, ast_node_list_t _names, ast_node_list_t _values) {
    ast_node_t* node = ast_node_new(AstLocalStatement, _position);
    node->array0 = _names;
    node->array1 = _values;
    return node;
}

DLLEXPORT ast_node_t* ast_if_statement_node(position_t* _position, ast_node_t* _cond, ast_node_t* tvalue, ast_node_t* fvalue) {
    ast_node_t* node = ast_node_new(AstIfStatement, _position);
    node->ast0 = _cond;
    node->ast1 = tvalue;
    node->ast2 = fvalue;
    return node;
}

DLLEXPORT ast_node_t* ast_return_node(position_t* _position, ast_node_t* _expr) {
    ast_node_t* node = ast_node_new(AstReturn, _position);
    node->ast0 = _expr;
    return node;
}

DLLEXPORT ast_node_t* ast_statement_expression_node(position_t* _position, ast_node_t* _expr) {
    ast_node_t* node = ast_node_new(AstStatementExpression, _position);
    node->ast0 = _expr;
    return node;
}

DLLEXPORT ast_node_t* ast_function_node(position_t* _position, ast_node_t* _name, ast_node_list_t _parameters, ast_node_list_t _body) {
    ast_node_t* node = ast_node_new(AstFunctionNode, _position);
    node->ast0 = _name;
    node->array0 = _parameters;
    node->array1 = _body;
    return node;
}

DLLEXPORT ast_node_t* ast_program_node(position_t* _position, ast_node_list_t _children) {
    ast_node_t* node = ast_node_new(AstProgram, _position);
    node->array0 = _children;
    return node;
}