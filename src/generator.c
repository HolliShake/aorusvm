#include "api/core/generator.h"
#include "api/core/vm.h"
#include "error.h"
#include "eval.h"
#include "internal.h"
#include "node.h"
#include "opcode.h"
#include "position.h"
#include "scope.h"

#ifndef GENERATOR_C
#define GENERATOR_C

typedef struct generator_struct {
    char*    fpath;
    char*    fdata;
    size_t   fsize;
    uint8_t* bytecode;
    size_t   bsize;
    size_t   reset_base;
} generator_t;


#define SAVEBASE(out) \
    size_t out = _generator->reset_base; \
    { _generator->reset_base = _generator->bsize; } \

#define RESTOREBASE(out) \
    _generator->reset_base = out; \

INTERNAL void generator_resize_bytecode_by(generator_t* _generator, size_t _size) {
    _generator->bytecode = (uint8_t*) realloc(_generator->bytecode, sizeof(uint8_t) * ((_generator->bsize + 1) + _size));
    ASSERTNULL(_generator->bytecode, ERROR_ALLOCATING_BYTECODE);
}


INTERNAL void generator_emit_raw_int(generator_t* _generator, size_t _value) {
    generator_resize_bytecode_by(_generator, 4);
    for (size_t i = 0; i < 4; i++) {
        _generator->bytecode[_generator->bsize++] = (_value >> (i * 8)) & 0xFF;
    }
}

INTERNAL void generator_allocate_nbytes(generator_t* _generator, size_t _nbytes) {
    generator_resize_bytecode_by(_generator, _nbytes);
    _generator->bsize += _nbytes;
}

INTERNAL void generator_emit_byte(generator_t* _generator, uint8_t _value) {
    generator_resize_bytecode_by(_generator, 1);
    _generator->bytecode[_generator->bsize++] = _value;
}

INTERNAL void generator_emit_magic_bytes(generator_t* _generator) {
    generator_resize_bytecode_by(_generator, 4);
    for (size_t i = 0; i < 4; i++) {
        _generator->bytecode[_generator->bsize++] = (MAGIC_NUMBER >> (i * 8)) & 0xFF;
    }
}

INTERNAL void generator_emit_version_bytes(generator_t* _generator) {
    generator_resize_bytecode_by(_generator, 4);
    for (size_t i = 0; i < 4; i++) {
        _generator->bytecode[_generator->bsize++] = (VERSION >> (i * 8)) & 0xFF;
    }
}

INTERNAL void generator_emit_bytecode_size(generator_t* _generator) {
    generator_resize_bytecode_by(_generator, 8);
    for (size_t i = 0; i < 8; i++) {
        _generator->bytecode[_generator->bsize++] = (_generator->bsize >> (i * 8)) & 0xFF;
    }
}

INTERNAL void generator_emit_raw_string(generator_t* _generator, char* _value) {
    generator_resize_bytecode_by(_generator, strlen(_value) + 1);
    for (size_t i = 0; _value[i] != '\0'; i++) {
        _generator->bytecode[_generator->bsize++] = _value[i];
    }
    _generator->bytecode[_generator->bsize++] = 0;
}

INTERNAL void generator_emit_int(generator_t* _generator, int _value) {
    generator_resize_bytecode_by(_generator, 5);
    _generator->bytecode[_generator->bsize++] = OPCODE_LOAD_INT;
    _generator->bytecode[_generator->bsize++] = _value & 0xFF;
    _generator->bytecode[_generator->bsize++] = (_value >>  8) & 0xFF;
    _generator->bytecode[_generator->bsize++] = (_value >> 16) & 0xFF;
    _generator->bytecode[_generator->bsize++] = (_value >> 24) & 0xFF;
}

INTERNAL void generator_emit_double(generator_t* _generator, double _value) {
    generator_resize_bytecode_by(_generator, 9);
    union double_bytes_t {
        double f64;
        uint8_t bytes[8];
    } double_bytes = {
        .f64 = _value,
    };
    _generator->bytecode[_generator->bsize++] = OPCODE_LOAD_DOUBLE;
    for (size_t i = 0; i < 8; i++) {
        _generator->bytecode[_generator->bsize++] = double_bytes.bytes[i];
    }
}

INTERNAL void generator_emit_string(generator_t* _generator, char* _value) {
    generator_resize_bytecode_by(_generator, strlen(_value) + 2);
    _generator->bytecode[_generator->bsize++] = OPCODE_LOAD_STRING;
    for (size_t i = 0; _value[i] != '\0'; i++) {
        _generator->bytecode[_generator->bsize++] = _value[i];
    }
    _generator->bytecode[_generator->bsize++] = 0;
}

INTERNAL void generator_set_4bytes(generator_t* _generator, size_t start, int _value) {
    for (size_t i = 0; i < 4; i++) {
        _generator->bytecode[start + i] = (_value >> (i * 8)) & 0xFF;
    }
}

INTERNAL void generator_set_8bytes(generator_t* _generator, size_t start, long _value) {
    for (size_t i = 0; i < 8; i++) {
        _generator->bytecode[start + i] = (_value >> (i * 8)) & 0xFF;
    }
}

INTERNAL bool generator_is_expression_type(ast_node_t* _expression) {
    switch (_expression->type) {
        case AstName:
        case AstInt:
        case AstLong:
        case AstFloat:
        case AstDouble:
        case AstBoolean:
        case AstString:
        case AstNull:
        case AstCall:
        case AstUnaryPlus:
        case AstBinaryMul:
        case AstBinaryDiv:
        case AstBinaryMod:
        case AstBinaryAdd:
        case AstBinarySub:
        case AstBinaryShl:
        case AstBinaryShr:
        case AstCmpLt:
        case AstCmpLte:
        case AstCmpGt:
        case AstCmpGte:
        case AstCmpEq:
        case AstCmpNe:
        case AstBinaryAnd:
        case AstBinaryOr:
        case AstBinaryXor:
        case AstLogicalAnd:
        case AstLogicalOr:
            return true;
        default:
            return false;
    }
}

INTERNAL bool generator_is_constant_node(ast_node_t* _expression) {
    switch (_expression->type) {
        case AstInt:
        case AstLong:
        case AstFloat:
        case AstDouble:
        case AstBoolean:
        case AstString:
        case AstNull:
            return true;
        case AstBinaryMul:
        case AstBinaryDiv:
        case AstBinaryMod:
        case AstBinaryAdd:
        case AstBinarySub:
        case AstBinaryShl:
        case AstBinaryShr:
        case AstLogicalAnd:
        case AstLogicalOr:
            return generator_is_constant_node(_expression->ast0) && generator_is_constant_node(_expression->ast1);
        default:
            return false;
    }
}

INTERNAL bool generator_is_logical_expression(ast_node_t* _expression) {
    return _expression->type == AstLogicalAnd || _expression->type == AstLogicalOr;
}

#define FOLD_CONSTANT_EXPRESSION(expression) { \
    eval_result_t result = eval_eval(expression); \
    switch (result.type) { \
        case EvalInt: \
            generator_emit_int(_generator, result.value.i32); \
            break; \
        case EvalDouble: \
            generator_emit_double(_generator, result.value.f64); \
            break; \
        case EvalBoolean: \
            generator_emit_byte(_generator, OPCODE_LOAD_BOOL); \
            generator_emit_byte(_generator, result.value.i32 == 1); \
            break; \
        case EvalString: \
            generator_emit_string(_generator, (char*) result.value.ptr); \
            break; \
        case EvalNull: \
            generator_emit_byte(_generator, OPCODE_LOAD_NULL); \
            break; \
        case EvalZeroDivision: \
            __THROW_ERROR( \
                _generator->fpath, \
                _generator->fdata, \
                expression->position, \
                "division by zero" \
            ); \
        case EvalError: \
        default: \
            __THROW_ERROR( \
                _generator->fpath, \
                _generator->fdata, \
                expression->position, \
                "unsupported type for expression", \
                result.type \
            ); \
    } \
}

INTERNAL void generator_assignment0(generator_t* _generator, ast_node_t* _expression) {
    switch (_expression->type) {
        case AstName:
            generator_emit_byte(_generator, OPCODE_LOAD_NAME);
            generator_emit_raw_string(
                _generator, 
                _expression->str0
            );
            break;
        default:
            __THROW_ERROR(
                _generator->fpath, 
                _generator->fdata, 
                _expression->position, 
                "assignment expression must be a name, but received %d", _expression->type
            );
    }
}

INTERNAL void generator_assignment1(generator_t* _generator, scope_t* _scope, ast_node_t* _expression) {
    switch (_expression->type) {
        case AstName:
            if (!scope_has(_scope, _expression->str0, true)) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "variable %s not found", _expression->str0
                );
            }
            scope_value_t symbol = scope_get(_scope, _expression->str0, true);
            if (symbol.is_const) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "constant variable %s cannot be re-assigned", _expression->str0
                );
            }
            generator_emit_byte(_generator, OPCODE_SET_NAME);
            generator_emit_raw_string(
                _generator, 
                _expression->str0
            );
            break;
        default:
            __THROW_ERROR(
                _generator->fpath, 
                _generator->fdata, 
                _expression->position, 
                "assignment expression must be a name, but received %d", _expression->type
            );
    }
}

INTERNAL void generator_expression(generator_t* _generator, scope_t* _scope, ast_node_t* _expression) {
    switch (_expression->type) {
        case AstName:
            generator_emit_byte(_generator, OPCODE_LOAD_NAME);
            generator_emit_raw_string(
                _generator, 
                _expression->str0
            );
            free(_expression->str0);
            free(_expression);
            break;
        case AstInt:
        case AstFloat:
            generator_emit_int(
                _generator, 
                (int) _expression->value.i32
            );
            free(_expression);
            break;
        case AstLong:
        case AstDouble:
            generator_emit_double(
                _generator, 
                (double) _expression->value.i64
            );
            free(_expression);
            break;
        case AstBoolean:
            generator_emit_byte(
                _generator, 
                OPCODE_LOAD_BOOL
            );
            generator_emit_byte(
                _generator, 
                _expression->value.i32 == 1
            );
            free(_expression);
            break;
        case AstString:
            if (_expression->str0 == NULL) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "string expression must have a value, but received NULL"
                );
            }
            generator_emit_string(
                _generator, 
                _expression->str0
            );
            free(_expression);
            break;
        case AstNull:
            generator_emit_byte(
                _generator, 
                OPCODE_LOAD_NULL
            );
            free(_expression);
            break;
        case AstCall: {
            if (_expression->ast0 == NULL) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "call expression must have a function, but received NULL"
                );
            }
            if (!generator_is_expression_type(_expression->ast0)) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "call expression must have a function, but received %d", _expression->ast0->type
                );
            }
            if (_expression->array0 == NULL) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "call expression must have arguments, but received NULL"
                );
            }
            ast_node_list_t arguments = _expression->array0;
            int param_count = 0;
            // Count arguments first
            for (param_count = 0; arguments[param_count] != NULL; param_count++);
            // Generate arguments right to left for stack
            for (int i = param_count - 1; i >= 0; i--) {
                ast_node_t* argument = arguments[i];
                if (!generator_is_expression_type(argument)) {
                    __THROW_ERROR(
                        _generator->fpath, 
                        _generator->fdata, 
                        argument->position,
                        "call expression must be an expression, but received %d", argument->type
                    );
                }
                generator_expression(_generator, _scope, argument);
            }
            generator_expression(_generator, _scope, _expression->ast0);
            generator_emit_byte(_generator, OPCODE_CALL);
            generator_emit_raw_int(_generator, param_count);
            free(_expression);
            break;
        }
        case AstUnaryPlus: {
            generator_assignment0(_generator, _expression->ast0);
            generator_emit_byte(_generator, OPCODE_INCREMENT);
            generator_assignment1(_generator, _scope, _expression->ast0);
            free(_expression);
            break;
        }
        case AstBinaryMul: {
            if (_expression->ast0 == NULL || _expression->ast1 == NULL) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "binary expression requires both left and right operands, but received NULL"
                );
            }
            if (!generator_is_expression_type(_expression->ast0) || !generator_is_expression_type(_expression->ast1)) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "binary expression requires both left and right operands to be expressions"
                );
            }
            if (generator_is_constant_node(_expression)) {
                FOLD_CONSTANT_EXPRESSION(_expression);
                return;
            }
            generator_expression(
                _generator, 
                _scope,
                _expression->ast0
            );
            generator_expression(
                _generator, 
                _scope,
                _expression->ast1
            );
            generator_emit_byte(_generator, OPCODE_MUL);
            free(_expression);
            break;
        }
        case AstBinaryDiv: {
            if (_expression->ast0 == NULL || _expression->ast1 == NULL) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "binary expression requires both left and right operands, but received NULL"
                );
            }
            if (!generator_is_expression_type(_expression->ast0) || !generator_is_expression_type(_expression->ast1)) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "binary expression requires both left and right operands to be expressions"
                );
            }
            if (generator_is_constant_node(_expression)) {
                FOLD_CONSTANT_EXPRESSION(_expression);
                return;
            }
            generator_expression(
                _generator, 
                _scope,
                _expression->ast0
            );
            generator_expression(
                _generator, 
                _scope,
                _expression->ast1
            );
            generator_emit_byte(_generator, OPCODE_DIV);
            free(_expression);
            break;
        }
        case AstBinaryMod: {
            if (_expression->ast0 == NULL || _expression->ast1 == NULL) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "binary expression requires both left and right operands, but received NULL"
                );
            }
            if (generator_is_constant_node(_expression)) {
                FOLD_CONSTANT_EXPRESSION(_expression);
                return;
            }
            generator_expression(
                _generator, 
                _scope,
                _expression->ast0
            );
            generator_expression(
                _generator, 
                _scope,
                _expression->ast1
            );
            generator_emit_byte(_generator, OPCODE_MOD);
            free(_expression);
            break;
        }
        case AstBinaryAdd: {
            if (_expression->ast0 == NULL || _expression->ast1 == NULL) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "binary expression requires both left and right operands, but received NULL"
                );
            }
            if (!generator_is_expression_type(_expression->ast0) || !generator_is_expression_type(_expression->ast1)) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "binary expression requires both left and right operands to be expressions"
                );
            }
            if (generator_is_constant_node(_expression)) {
                FOLD_CONSTANT_EXPRESSION(_expression);
                return;
            }
            generator_expression(
                _generator, 
                _scope,
                _expression->ast0
            );
            generator_expression(
                _generator, 
                _scope,
                _expression->ast1
            );
            generator_emit_byte(_generator, OPCODE_ADD);
            free(_expression);
            break;
        }
        case AstBinarySub: {
            if (_expression->ast0 == NULL || _expression->ast1 == NULL) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "binary expression requires both left and right operands, but received NULL"
                );
            }
            if (!generator_is_expression_type(_expression->ast0) || !generator_is_expression_type(_expression->ast1)) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "binary expression requires both left and right operands to be expressions"
                );
            }
            if (generator_is_constant_node(_expression)) {
                FOLD_CONSTANT_EXPRESSION(_expression);
                return;
            }
            generator_expression(
                _generator, 
                _scope,
                _expression->ast0
            );
            generator_expression(
                _generator, 
                _scope,
                _expression->ast1
            );
            generator_emit_byte(_generator, OPCODE_SUB);
            free(_expression);
            break;
        }
        case AstBinaryShl: {
            if (_expression->ast0 == NULL || _expression->ast1 == NULL) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "binary expression requires both left and right operands, but received NULL"
                );
            }
            if (!generator_is_expression_type(_expression->ast0) || !generator_is_expression_type(_expression->ast1)) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "binary expression requires both left and right operands to be expressions"
                );
            }
            if (generator_is_constant_node(_expression)) {
                FOLD_CONSTANT_EXPRESSION(_expression);
                return;
            }
            generator_expression(
                _generator, 
                _scope,
                _expression->ast0
            );
            generator_expression(
                _generator, 
                _scope,
                _expression->ast1
            );
            generator_emit_byte(_generator, OPCODE_SHL);
            free(_expression);
            break;
        }
        case AstBinaryShr: {
            if (_expression->ast0 == NULL || _expression->ast1 == NULL) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "binary expression requires both left and right operands, but received NULL"
                );
            }
            if (!generator_is_expression_type(_expression->ast0) || !generator_is_expression_type(_expression->ast1)) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "binary expression requires both left and right operands to be expressions"
                );
            }
            if (generator_is_constant_node(_expression)) {
                FOLD_CONSTANT_EXPRESSION(_expression);
                return;
            }
            generator_expression(
                _generator, 
                _scope,
                _expression->ast0
            );
            generator_expression(
                _generator, 
                _scope,
                _expression->ast1
            );
            generator_emit_byte(_generator, OPCODE_SHR);
            free(_expression);
            break;
        }
        case AstCmpLt: {
            if (_expression->ast0 == NULL || _expression->ast1 == NULL) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "binary expression requires both left and right operands, but received NULL"
                );
            }
            if (!generator_is_expression_type(_expression->ast0) || !generator_is_expression_type(_expression->ast1)) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "binary expression requires both left and right operands to be expressions"
                );
            }
            if (generator_is_constant_node(_expression)) {
                FOLD_CONSTANT_EXPRESSION(_expression);
                return;
            }
            generator_expression(
                _generator, 
                _scope,
                _expression->ast0
            );
            generator_expression(
                _generator, 
                _scope,
                _expression->ast1
            );
            generator_emit_byte(_generator, OPCODE_CMP_LT);
            free(_expression);
            break;
        }
        case AstCmpLte: {
            if (_expression->ast0 == NULL || _expression->ast1 == NULL) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "binary expression requires both left and right operands, but received NULL"
                );
            }
            if (generator_is_constant_node(_expression)) {
                FOLD_CONSTANT_EXPRESSION(_expression);
                return;
            }
            generator_expression(
                _generator, 
                _scope,
                _expression->ast0
            );
            generator_expression(
                _generator, 
                _scope,
                _expression->ast1
            );
            generator_emit_byte(_generator, OPCODE_CMP_LTE);
            free(_expression);
            break;
        }
        case AstCmpGt: {
            if (_expression->ast0 == NULL || _expression->ast1 == NULL) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "binary expression requires both left and right operands, but received NULL"
                );
            }
            if (generator_is_constant_node(_expression)) {
                FOLD_CONSTANT_EXPRESSION(_expression);
                return;
            }
            generator_expression(
                _generator, 
                _scope,
                _expression->ast0
            );
            generator_expression(
                _generator, 
                _scope,
                _expression->ast1
            );
            generator_emit_byte(_generator, OPCODE_CMP_GT);
            free(_expression);
            break;
        }
        case AstCmpGte: {
            if (_expression->ast0 == NULL || _expression->ast1 == NULL) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "binary expression requires both left and right operands, but received NULL"
                );
            }
            if (generator_is_constant_node(_expression)) {
                FOLD_CONSTANT_EXPRESSION(_expression);
                return;
            }
            generator_expression(
                _generator, 
                _scope,
                _expression->ast0
            );
            generator_expression(
                _generator, 
                _scope,
                _expression->ast1
            );
            generator_emit_byte(_generator, OPCODE_CMP_GTE);
            free(_expression);
            break;
        }
        case AstCmpEq: {
            if (_expression->ast0 == NULL || _expression->ast1 == NULL) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "binary expression requires both left and right operands, but received NULL"
                );
            }
            if (generator_is_constant_node(_expression)) {
                FOLD_CONSTANT_EXPRESSION(_expression);
                return;
            }
            generator_expression(
                _generator, 
                _scope,
                _expression->ast0
            );
            generator_expression(
                _generator, 
                _scope,
                _expression->ast1
            );
            generator_emit_byte(_generator, OPCODE_CMP_EQ);
            free(_expression);
            break;
        }
        case AstCmpNe: {
            if (_expression->ast0 == NULL || _expression->ast1 == NULL) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "binary expression requires both left and right operands, but received NULL"
                );
            }
            if (generator_is_constant_node(_expression)) {
                FOLD_CONSTANT_EXPRESSION(_expression);
                return;
            }
            generator_expression(
                _generator, 
                _scope,
                _expression->ast0
            );
            generator_expression(
                _generator, 
                _scope,
                _expression->ast1
            );
            generator_emit_byte(_generator, OPCODE_CMP_NE);
            free(_expression);
            break;
        }
        case AstLogicalAnd: {
            if (_expression->ast0 == NULL || _expression->ast1 == NULL) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "logical expression requires both left and right operands, but received NULL"
                );
            }
            if (!generator_is_expression_type(_expression->ast0) || !generator_is_expression_type(_expression->ast1)) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "binary expression requires both left and right operands to be expressions"
                );
            }
            if (generator_is_constant_node(_expression)) {
                FOLD_CONSTANT_EXPRESSION(_expression);
                return;
            }
            generator_expression(
                _generator, 
                _scope,
                _expression->ast0
            );
            generator_emit_byte(_generator, OPCODE_JUMP_IF_FALSE_OR_POP);
            int jump_start = _generator->bsize;
            generator_allocate_nbytes(_generator, 4);
            generator_expression(
                _generator, 
                _scope,
                _expression->ast1
            );
            generator_set_4bytes(_generator, jump_start, _generator->bsize - jump_start - _generator->reset_base);
            free(_expression);
            break;
        }
        case AstLogicalOr: {
            if (_expression->ast0 == NULL || _expression->ast1 == NULL) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "logical expression requires both left and right operands, but received NULL"
                );
            }
            if (!generator_is_expression_type(_expression->ast0) || !generator_is_expression_type(_expression->ast1)) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "binary expression requires both left and right operands to be expressions"
                );
            }
            if (generator_is_constant_node(_expression)) {
                FOLD_CONSTANT_EXPRESSION(_expression);
                return;
            }
            generator_expression(
                _generator, 
                _scope,
                _expression->ast0
            );
            generator_emit_byte(_generator, OPCODE_JUMP_IF_TRUE_OR_POP);
            int jump_start = _generator->bsize;
            generator_allocate_nbytes(_generator, 4);
            generator_expression(
                _generator, 
                _scope,
                _expression->ast1
            );
            generator_set_4bytes(_generator, jump_start, _generator->bsize - jump_start - _generator->reset_base);
            free(_expression);
            break;
        }
        default:
            PD("unsupported expression type %d, but received %d", _expression->type, _expression->type);
    }
}

INTERNAL void generator_statement(generator_t* _generator, scope_t* _scope, ast_node_t* _statement) {
    switch (_statement->type) {
        case AstVarStatement:
        case AstConstStatement:
        case AstLocalStatement: {
            // Validate scope
            if (_statement->type == AstVarStatement && !scope_is_global(_scope)) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _statement->position, 
                    "global variable declaration must be in the global scope"
                );
            }
            if (_statement->type == AstLocalStatement && !scope_is_local(_scope)) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _statement->position, 
                    "local variable declaration must be in a function or block"
                );
            }
            
            ast_node_list_t names  = _statement->array0;
            ast_node_list_t values = _statement->array1;

            if (names == NULL) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _statement->position, 
                    "variable declaration must have names"
                );
            }
            if (values == NULL) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _statement->position, 
                    "variable declaration must have values"
                );
            }
            for (size_t i = 0; names[i] != NULL; i++) {
                ast_node_t* name = names[i];
                ast_node_t* value = values[i];

                // Validate name and value
                if (name && name->type != AstName) {
                    __THROW_ERROR(
                        _generator->fpath, 
                        _generator->fdata, 
                        _statement->position, 
                        "variable name must be a valid identifier, but received %d", name->type
                    );
                }
                if (value && !generator_is_expression_type(value)) {
                    __THROW_ERROR(
                        _generator->fpath, 
                        _generator->fdata, 
                        _statement->position, 
                        "variable value must be an expression"
                    );
                }
                if (scope_has(_scope, name->str0, false)) {
                    __THROW_ERROR(
                        _generator->fpath, 
                        _generator->fdata, 
                        _statement->position, 
                        "symbol %s is already defined", name->str0
                    );
                    return;
                }
                // Generate value code
                if (value) {
                    generator_expression(_generator, _scope, value);
                } else {
                    generator_emit_byte(_generator, OPCODE_LOAD_NULL);
                }

                // Store value
                generator_emit_byte(_generator, OPCODE_STORE_NAME);
                generator_emit_raw_string(_generator, name->str0);

                // Add to scope
                scope_value_t symbol = {
                    .name      = name->str0,
                    .is_const  = (_statement->type == AstConstStatement),
                    .is_global = scope_is_global(_scope),
                    .position  = name->position
                };
                scope_put(_scope, name->str0, symbol);
                free(name);
            }
            free(names);
            free(values);
            free(_statement);
            break;
        }
        case AstIfStatement: {
            ast_node_t* cond   = _statement->ast0;
            ast_node_t* tvalue = _statement->ast1;
            ast_node_t* fvalue = _statement->ast2;
            if (!cond || !tvalue || !generator_is_expression_type(cond)) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _statement->position, 
                    "if statement must have a condition, a true value, and a false value"
                );
            }
            if (generator_is_constant_node(cond) || !generator_is_logical_expression(cond)) {
                // Condition
                generator_expression(_generator, _scope, cond);
                // Jump if false
                generator_emit_byte(_generator, OPCODE_POP_JUMP_IF_FALSE);
                int jump_start = _generator->bsize;
                generator_allocate_nbytes(_generator, 4);
                // true
                generator_statement(_generator, _scope, tvalue);
                // Jump to endif from true
                generator_emit_byte(_generator, OPCODE_JUMP_FORWARD);
                int jump_endif_from_true = _generator->bsize;
                generator_allocate_nbytes(_generator, 4);
                // false?
                generator_set_4bytes(_generator, jump_start, _generator->bsize - jump_start);
                if (fvalue != NULL) {
                    generator_statement(_generator, _scope, fvalue);
                }
                // Jump to endif from false
                generator_emit_byte(_generator, OPCODE_JUMP_FORWARD);
                int jump_endif_from_false = _generator->bsize;
                generator_allocate_nbytes(_generator, 4);
                generator_set_4bytes(_generator, jump_endif_from_true , _generator->bsize - jump_endif_from_true);
                generator_set_4bytes(_generator, jump_endif_from_false, _generator->bsize - jump_endif_from_false);
                free(_statement);
                break;
            } else {
                ast_node_t* cond_l = cond->ast0;
                ast_node_t* cond_r = cond->ast1;
                if (!cond_l || !cond_r || !generator_is_logical_expression(cond)) {
                    __THROW_ERROR(
                        _generator->fpath, 
                        _generator->fdata, 
                        cond->position, 
                        "logical expression must have a left and right operand"
                    );
                }
                if (!generator_is_expression_type(cond_l) || !generator_is_expression_type(cond_r)) {
                    __THROW_ERROR(
                        _generator->fpath, 
                        _generator->fdata, 
                        cond->position, 
                        "logical expression must have both left and right operands to be expressions"
                    );
                }
                bool is_logical_and = cond->type == AstLogicalAnd;
                if (is_logical_and) {
                    generator_expression(_generator, _scope, cond_l);
                    generator_emit_byte(_generator, OPCODE_POP_JUMP_IF_FALSE);
                    int jump_start_l = _generator->bsize;
                    generator_allocate_nbytes(_generator, 4);
                    // If left is true, then evaluate right
                    generator_expression(_generator, _scope, cond_r);
                    generator_emit_byte(_generator, OPCODE_POP_JUMP_IF_FALSE);
                    int jump_start_r = _generator->bsize;
                    generator_allocate_nbytes(_generator, 4);
                    // true
                    generator_statement(_generator, _scope, tvalue);
                    // Jump to endif from true
                    generator_emit_byte(_generator, OPCODE_JUMP_FORWARD);
                    int jump_endif_from_true = _generator->bsize;
                    generator_allocate_nbytes(_generator, 4);
                    // false?
                    generator_set_4bytes(_generator, jump_start_l, _generator->bsize - jump_start_l);
                    generator_set_4bytes(_generator, jump_start_r, _generator->bsize - jump_start_r);
                    if (fvalue != NULL) {
                        generator_statement(_generator, _scope, fvalue);
                    }
                    // Jump to endif from false
                    generator_emit_byte(_generator, OPCODE_JUMP_FORWARD);
                    int jump_endif_from_false = _generator->bsize;
                    generator_allocate_nbytes(_generator, 4);
                    generator_set_4bytes(_generator, jump_endif_from_true , _generator->bsize - jump_endif_from_true );
                    generator_set_4bytes(_generator, jump_endif_from_false, _generator->bsize - jump_endif_from_false);
                    free(cond);
                    free(_statement);
                } else {
                    generator_expression(_generator, _scope, cond_l);
                    generator_emit_byte(_generator, OPCODE_POP_JUMP_IF_TRUE);
                    int jump_start_l = _generator->bsize;
                    generator_allocate_nbytes(_generator, 4);
                    // If left is false, then evaluate right
                    generator_expression(_generator, _scope, cond_r);
                    generator_emit_byte(_generator, OPCODE_POP_JUMP_IF_FALSE);
                    int jump_start_r = _generator->bsize;
                    generator_allocate_nbytes(_generator, 4);
                    // true
                    generator_set_4bytes(_generator, jump_start_l, _generator->bsize - jump_start_l);
                    generator_statement(_generator, _scope, tvalue);
                    // Jump to endif from true
                    generator_emit_byte(_generator, OPCODE_JUMP_FORWARD);
                    int jump_endif_from_true = _generator->bsize;
                    generator_allocate_nbytes(_generator, 4);
                    // false?
                    generator_set_4bytes(_generator, jump_start_r, _generator->bsize - jump_start_r);
                    if (fvalue != NULL) {
                        generator_statement(_generator, _scope, fvalue);
                    }
                    // Jump to endif from false
                    generator_emit_byte(_generator, OPCODE_JUMP_FORWARD);
                    int jump_endif_from_false = _generator->bsize;
                    generator_allocate_nbytes(_generator, 4);
                    generator_set_4bytes(_generator, jump_endif_from_true , _generator->bsize - jump_endif_from_true );
                    generator_set_4bytes(_generator, jump_endif_from_false, _generator->bsize - jump_endif_from_false);
                    free(cond);
                    free(_statement);
                }
            }
            break;
        }
        case AstWhileStatement: {
            ast_node_t* cond = _statement->ast0;
            ast_node_t* body = _statement->ast1;
            if (!cond || !body || !generator_is_expression_type(cond)) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _statement->position, 
                    "while statement must have a condition and a body"
                );
            }
            scope_t* while_scope = scope_new(_scope, ScopeTypeLoop);
            size_t loop_start = _generator->bsize;
            if (generator_is_constant_node(cond) || !generator_is_logical_expression(cond)) {
                // Condition
                generator_expression(_generator, _scope, cond);
                // Jump if false
                generator_emit_byte(_generator, OPCODE_POP_JUMP_IF_FALSE);
                int jump_endwhile_if_false = _generator->bsize;
                generator_allocate_nbytes(_generator, 4);
                // Body
                generator_statement(_generator, while_scope, body);
                // Jump to the start of the while loop
                generator_emit_byte(_generator, OPCODE_ABSOLUTE_JUMP);
                generator_emit_raw_int(_generator, loop_start);
                // Jump to the end of the while loop
                generator_set_4bytes(_generator, jump_endwhile_if_false, _generator->bsize - jump_endwhile_if_false);
                scope_free(while_scope);
                free(_statement);
            } else {
                ast_node_t* cond_l = cond->ast0;
                ast_node_t* cond_r = cond->ast1;
                if (!cond_l || !cond_r || !generator_is_logical_expression(cond)) {
                    __THROW_ERROR(
                        _generator->fpath, 
                        _generator->fdata, 
                        cond->position, 
                        "logical expression must have a left and right operand"
                    );
                }
                if (!generator_is_expression_type(cond_l) || !generator_is_expression_type(cond_r)) {
                    __THROW_ERROR(
                        _generator->fpath, 
                        _generator->fdata, 
                        cond->position, 
                        "logical expression must have both left and right operands to be expressions"
                    );
                }
                bool is_logical_and = cond->type == AstLogicalAnd;
                if (is_logical_and) {
                    generator_expression(_generator, _scope, cond_l);
                    generator_emit_byte(_generator, OPCODE_POP_JUMP_IF_FALSE);
                    int jump_start_l = _generator->bsize;
                    generator_allocate_nbytes(_generator, 4);
                    // If left is true, then evaluate right
                    generator_expression(_generator, _scope, cond_r);
                    generator_emit_byte(_generator, OPCODE_POP_JUMP_IF_FALSE);
                    int jump_start_r = _generator->bsize;
                    generator_allocate_nbytes(_generator, 4);
                    // true
                    generator_statement(_generator, while_scope, body);
                    // Jump to the start of the while loop
                    generator_emit_byte(_generator, OPCODE_ABSOLUTE_JUMP);
                    generator_emit_raw_int(_generator, loop_start);
                    // Jump to the end of the while loop
                    generator_set_4bytes(_generator, jump_start_l, _generator->bsize - jump_start_l);
                    generator_set_4bytes(_generator, jump_start_r, _generator->bsize - jump_start_r);
                    scope_free(while_scope);
                    free(_statement);
                } else {
                    generator_expression(_generator, _scope, cond_l);
                    generator_emit_byte(_generator, OPCODE_POP_JUMP_IF_TRUE);
                    int jump_start_l = _generator->bsize;
                    generator_allocate_nbytes(_generator, 4);
                    // If left is false, then evaluate right
                    generator_expression(_generator, _scope, cond_r);
                    generator_emit_byte(_generator, OPCODE_POP_JUMP_IF_FALSE);
                    int jump_start_r = _generator->bsize;
                    generator_allocate_nbytes(_generator, 4);
                    // Set if true
                    generator_set_4bytes(_generator, jump_start_l, _generator->bsize - jump_start_l);
                    // true
                    generator_statement(_generator, while_scope, body);
                    // Jump to the start of the while loop
                    generator_emit_byte(_generator, OPCODE_ABSOLUTE_JUMP);
                    generator_emit_raw_int(_generator, loop_start);
                    // Set if false
                    generator_set_4bytes(_generator, jump_start_r, _generator->bsize - jump_start_r);
                    scope_free(while_scope);
                    free(_statement);
                }
            }
            break;
        }
        case AstReturnStatement: {
            if (!scope_is_function(_scope)) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _statement->position, 
                    "return statement must be in a function"
                );
            }
            // Find function scope and set returned flag
            scope_t* current;
            for (current = _scope; current->type != ScopeTypeFunction; current = current->parent);
            current->is_returned = true;
            // Generate return value and return opcode
            ast_node_t* expr = _statement->ast0;
            if (expr != NULL && !generator_is_expression_type(expr)) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _statement->position,
                    "return value must be an expression"
                );
            }
            if (expr != NULL) {
                generator_expression(_generator, _scope, expr);
            } else {
                generator_emit_byte(_generator, OPCODE_LOAD_NULL);
            }
            generator_emit_byte(_generator, OPCODE_RETURN);
            free(_statement);
            break;
        }
        case AstExpressionStatement: {
            if (_statement->ast0 == NULL) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _statement->position, 
                    "expression statement must have an expression, but received NULL"
                );
            }
            if (!generator_is_expression_type(_statement->ast0)) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _statement->position, 
                    "expression statement must be an expression, but received %d", _statement->ast0->type
                );
            }
            generator_expression(_generator, _scope, _statement->ast0);
            generator_emit_byte(_generator, OPCODE_POPTOP);
            free(_statement);
            break;
        }
        case AstFunctionNode: 
        case AstAsyncFunctionNode: {
            bool is_async = _statement->type == AstAsyncFunctionNode;
            ast_node_t* name       = _statement->ast0;
            ast_node_list_t params = _statement->array0;
            ast_node_list_t body   = _statement->array1;
            // Validate scope
            if (!scope_is_global(_scope)) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _statement->position, 
                    "function declaration must be in the global scope"
                );
            }
            // Validate name
            if (name->type != AstName) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _statement->position, 
                    "function name must be a valid identifier"
                );
            }
            if (params == NULL) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _statement->position, 
                    "function must have parameters, but received NULL"
                );
            }
            if (body == NULL) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _statement->position, 
                    "function must have a body, but received NULL"
                );
            }
            if (scope_has(_scope, name->str0, true)) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _statement->position, 
                    "function %s is already defined", name->str0
                );
            }
            // Make function
            generator_emit_byte(_generator, is_async ? OPCODE_MAKE_ASYNC_FUNCTION : OPCODE_MAKE_FUNCTION);
            // Reserve n bytes for the parameter count
            size_t param_count;
            for (param_count = 0; params[param_count] != NULL; param_count++);
            // Reserve n bytes for the parameter count
            size_t function_start = _generator->bsize;
            generator_emit_raw_int(_generator, param_count);
            // Reserve 8 bytes for the bytecode size
            size_t size_address = _generator->bsize;
            // Reserve 8 bytes for the bytecode size
            generator_emit_bytecode_size(_generator);
            // Reserve n bytes for the file name
            generator_emit_raw_string(_generator, _generator->fpath);
            // Reserve n bytes for the module name
            generator_emit_raw_string(_generator, name->str0);
            // Create function scope
            scope_t* function_scope = scope_new(_scope, ScopeTypeFunction);
            scope_t* local_scope = scope_new(function_scope, ScopeTypeLocal);
            // Compile parameters
            for (size_t i = 0; params[i] != NULL; i++) {
                ast_node_t* param = params[i];
                if (param->type != AstName) {
                    __THROW_ERROR(
                        _generator->fpath, 
                        _generator->fdata, 
                        param->position, 
                        "function parameter must be a valid identifier, but received %d", param->type
                    );
                }
                generator_emit_byte(_generator, OPCODE_STORE_NAME);
                generator_emit_raw_string(_generator, param->str0);
                free(param);
            }
            // Compile body
            for (size_t i = 0; body[i] != NULL; i++) {
                ast_node_t* statement = body[i];
                if (generator_is_expression_type(statement)) {
                    __THROW_ERROR(
                        _generator->fpath, 
                        _generator->fdata, 
                        statement->position, 
                        "function body must contain statement only, but received %d", statement->type
                    );
                }
                generator_statement(_generator, local_scope, statement);
            }
            // Emit the return opcode
            generator_emit_byte(_generator, OPCODE_LOAD_NULL);
            generator_emit_byte(_generator, OPCODE_RETURN);
            // Save into symbol table
            scope_value_t symbol = {
                .name      = name->str0,
                .is_const  = false,
                .is_global = true,
                .position  = name->position
            };
            scope_put(_scope, name->str0, symbol);
            // Set the bytecode size
            generator_set_8bytes(_generator, size_address, _generator->bsize - function_start);
            generator_emit_byte(_generator, OPCODE_STORE_NAME);
            generator_emit_raw_string(_generator, name->str0);
            // Free the function scope
            scope_free(local_scope);
            scope_free(function_scope);
            free(name);
            free(params);
            free(body);
            free(_statement);
            break;
        }
        case AstBlockStatement: {
            ast_node_list_t statements = _statement->array0;
            scope_t* block_scope = scope_new(_scope, ScopeTypeLocal);
            // Setup block and reserve space for metadata
            generator_emit_byte(_generator, OPCODE_SETUP_BLOCK);
            size_t size_address = _generator->bsize;
            generator_emit_bytecode_size(_generator);
            generator_emit_raw_string(_generator, _generator->fpath);
            generator_emit_raw_string(_generator, "block");
            // Compile all statements in block
            for (ast_node_t** stmt = statements; *stmt; stmt++) {
                generator_statement(_generator, block_scope, *stmt);
            }
            // Emit complete and finalize
            generator_emit_byte(_generator, OPCODE_LOAD_NULL);
            generator_emit_byte(_generator, OPCODE_COMPLETE_BLOCK);
            generator_set_8bytes(_generator, size_address, _generator->bsize - size_address);
            // Cleanup
            scope_free(block_scope);
            free(statements);
            free(_statement);
            break;
        }
        default:
            PD("unsupported statement type %d.", _statement->type);
    }
}

INTERNAL void generator_program(generator_t* _generator, ast_node_t* _program) {
    ast_node_list_t children = _program->array0;
    // Reserve 4 bytes for the magic number
    generator_emit_magic_bytes(_generator);
    // Reserve 4 bytes for the version
    generator_emit_version_bytes(_generator);
    // Reserve 8 bytes for the bytecode size
    generator_emit_bytecode_size(_generator);
    // Reserve n bytes for the file name
    generator_emit_raw_string(_generator, _generator->fpath);
    // Reserve n bytes for the module name
    generator_emit_raw_string(_generator, path_get_file_name(_generator->fpath));
    // Emit the bytecode
    scope_t* scope = scope_new(NULL, ScopeTypeGlobal);
    for (size_t i = 0; children[i] != NULL; i++) {
        ast_node_t* child = children[i];
        if (generator_is_expression_type(child)) {
            __THROW_ERROR(
                _generator->fpath, 
                _generator->fdata, 
                child->position, 
                "program must contain statements only, but received %d", child->type
            );
        }
        generator_statement(_generator, scope, child);
    }
    // write the bytecode to the file
    generator_emit_byte(_generator, OPCODE_LOAD_NULL);
    generator_emit_byte(_generator, OPCODE_RETURN);
    // Set the bytecode size
    generator_set_8bytes(_generator, 8, _generator->bsize);
    // Free the children and the program
    free(children);
    free(_program);
    scope_free(scope);
}

// -----------------------------

DLLEXPORT generator_t* generator_new(char* _fpath, char* _fdata) {
    generator_t* generator = (generator_t*) malloc(sizeof(generator_t));
    ASSERTNULL(generator, ERROR_ALLOCATING_GENERATOR);
    generator->fpath = string_allocate(_fpath);
    generator->fdata = string_allocate(_fdata);
    generator->fsize = strlen(_fdata);
    generator->bsize = 0;
    generator->reset_base = 0;
    generator->bytecode = (uint8_t*) malloc(sizeof(uint8_t) * 1);
    ASSERTNULL(generator->bytecode, ERROR_ALLOCATING_BYTECODE);
    // Return instance
    return generator;
}

DLLEXPORT uint8_t* generator_generate(generator_t* _generator, ast_node_t* _program) {
    generator_program(_generator, _program);
    return _generator->bytecode;
}

DLLEXPORT void generator_free(generator_t* _generator) {
    free(_generator->fpath);
    free(_generator->fdata);
    free(_generator);
}

#endif