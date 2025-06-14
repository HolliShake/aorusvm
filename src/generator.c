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
} generator_t;

INTERNAL void generator_resize_bytecode_by(generator_t* _generator, size_t _size) {
    _generator->bytecode = (uint8_t*) realloc(_generator->bytecode, sizeof(uint8_t) * ((_generator->bsize + 1) + _size));
    ASSERTNULL(_generator->bytecode, ERROR_ALLOCATING_BYTECODE);
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

INTERNAL void generator_emit_long(generator_t* _generator, long _value) {
    generator_resize_bytecode_by(_generator, 9);
    _generator->bytecode[_generator->bsize++] = OPCODE_LOAD_LONG;
    _generator->bytecode[_generator->bsize++] = _value & 0xFF;
    _generator->bytecode[_generator->bsize++] = (_value >>  8) & 0xFF;
    _generator->bytecode[_generator->bsize++] = (_value >> 16) & 0xFF;
    _generator->bytecode[_generator->bsize++] = (_value >> 24) & 0xFF;
    _generator->bytecode[_generator->bsize++] = (_value >> 32) & 0xFF;
    _generator->bytecode[_generator->bsize++] = (_value >> 40) & 0xFF;
    _generator->bytecode[_generator->bsize++] = (_value >> 48) & 0xFF;
    _generator->bytecode[_generator->bsize++] = (_value >> 56) & 0xFF;
}

INTERNAL void generator_emit_float(generator_t* _generator, float _value) {
    generator_resize_bytecode_by(_generator, 5);
    union float_bytes_t {
        float f32;
        uint8_t bytes[4];
    } float_bytes = {
        .f32 = _value,
    };
    _generator->bytecode[_generator->bsize++] = OPCODE_LOAD_FLOAT;
    for (size_t i = 0; i < 4; i++) {
        _generator->bytecode[_generator->bsize++] = float_bytes.bytes[i];
    }
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
        case AstBinaryMul:
        case AstBinaryDiv:
        case AstBinaryMod:
        case AstBinaryAdd:
        case AstBinarySub:
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
        case AstBinaryAdd:
        case AstBinarySub:
        case AstBinaryMul:
        case AstBinaryDiv:
        case AstBinaryMod:
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
        case EvalLong: \
            generator_emit_long(_generator, result.value.i64); \
            break; \
        case EvalFloat: \
            generator_emit_float(_generator, result.value.f32); \
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

INTERNAL void generator_expression(generator_t* _generator, ast_node_t* _expression) {
    switch (_expression->type) {
        case AstName:
            generator_emit_byte(_generator, OPCODE_LOAD_NAME);
            generator_emit_raw_string(
                _generator, 
                _expression->str0
            );
            free(_expression);
            break;
        case AstInt:
            generator_emit_int(
                _generator, 
                _expression->value.i32
            );
            free(_expression);
            break;
        case AstLong:
            generator_emit_long(
                _generator, 
                _expression->value.i64
            );
            free(_expression);
            break;
        case AstFloat:
            generator_emit_float(
                _generator, 
                _expression->value.f32
            );
            free(_expression);
            break;
        case AstDouble:
            generator_emit_double(
                _generator, 
                _expression->value.f64
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
                PD("string expression must have a value, but received NULL");
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
        case AstBinaryMul: {
            if (_expression->ast0 == NULL || _expression->ast1 == NULL) {
                PD("binary expression requires both left and right operands, but received NULL");
            }
            if (generator_is_constant_node(_expression)) {
                FOLD_CONSTANT_EXPRESSION(_expression);
                return;
            }
            generator_expression(
                _generator, 
                _expression->ast0
            );
            generator_expression(
                _generator, 
                _expression->ast1
            );
            generator_emit_byte(_generator, OPCODE_MUL);
            free(_expression);
            break;
        }
        case AstBinaryDiv: {
            if (_expression->ast0 == NULL || _expression->ast1 == NULL) {
                PD("binary expression requires both left and right operands, but received NULL");
            }
            if (generator_is_constant_node(_expression)) {
                FOLD_CONSTANT_EXPRESSION(_expression);
                return;
            }
            generator_expression(
                _generator, 
                _expression->ast0
            );
            generator_expression(
                _generator, 
                _expression->ast1
            );
            generator_emit_byte(_generator, OPCODE_DIV);
            free(_expression);
            break;
        }
        case AstBinaryMod: {
            if (_expression->ast0 == NULL || _expression->ast1 == NULL) {
                PD("binary expression requires both left and right operands, but received NULL");
            }
            if (generator_is_constant_node(_expression)) {
                FOLD_CONSTANT_EXPRESSION(_expression);
                return;
            }
            generator_expression(
                _generator, 
                _expression->ast0
            );
            generator_expression(
                _generator, 
                _expression->ast1
            );
            generator_emit_byte(_generator, OPCODE_MOD);
            free(_expression);
            break;
        }
        case AstBinaryAdd: {
            if (_expression->ast0 == NULL || _expression->ast1 == NULL) {
                PD("binary expression requires both left and right operands, but received NULL");
            }
            if (generator_is_constant_node(_expression)) {
                FOLD_CONSTANT_EXPRESSION(_expression);
                return;
            }
            generator_expression(
                _generator, 
                _expression->ast0
            );
            generator_expression(
                _generator, 
                _expression->ast1
            );
            generator_emit_byte(_generator, OPCODE_ADD);
            free(_expression);
            break;
        }
        case AstBinarySub: {
            if (_expression->ast0 == NULL || _expression->ast1 == NULL) {
                PD("binary expression requires both left and right operands, but received NULL");
            }
            if (generator_is_constant_node(_expression)) {
                FOLD_CONSTANT_EXPRESSION(_expression);
                return;
            }
            generator_expression(
                _generator, 
                _expression->ast0
            );
            generator_expression(
                _generator, 
                _expression->ast1
            );
            generator_emit_byte(_generator, OPCODE_SUB);
            free(_expression);
            break;
        }
        case AstLogicalAnd: {
            if (_expression->ast0 == NULL || _expression->ast1 == NULL) {
                PD("logical expression requires both left and right operands, but received NULL");
            }
            if (generator_is_constant_node(_expression)) {
                FOLD_CONSTANT_EXPRESSION(_expression);
                return;
            }
            generator_expression(
                _generator, 
                _expression->ast0
            );
            generator_emit_byte(_generator, OPCODE_JUMP_IF_FALSE_OR_POP);
            int jump_start = _generator->bsize;
            generator_allocate_nbytes(_generator, 4);
            generator_expression(
                _generator, 
                _expression->ast1
            );
            generator_set_4bytes(_generator, jump_start, _generator->bsize - jump_start);
            free(_expression);
            break;
        }
        case AstLogicalOr: {
            if (_expression->ast0 == NULL || _expression->ast1 == NULL) {
                PD("logical expression requires both left and right operands, but received NULL");
            }
            if (generator_is_constant_node(_expression)) {
                FOLD_CONSTANT_EXPRESSION(_expression);
                return;
            }
            generator_expression(
                _generator, 
                _expression->ast0
            );
            generator_emit_byte(_generator, OPCODE_JUMP_IF_TRUE_OR_POP);
            int jump_start = _generator->bsize;
            generator_allocate_nbytes(_generator, 4);
            generator_expression(
                _generator, 
                _expression->ast1
            );
            generator_set_4bytes(_generator, jump_start, _generator->bsize - jump_start);
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

            for (size_t i = 0; names[i] != NULL; i++) {
                ast_node_t* name = names[i];
                ast_node_t* value = values[i];

                // Validate name and value
                if (!name || name->type != AstName) {
                    if (!name) PD("variable name must be a valid identifier, but received NULL");
                    __THROW_ERROR(
                        _generator->fpath, 
                        _generator->fdata, 
                        name->position, 
                        "variable name must be a valid identifier"
                    );
                }
                if (value && !generator_is_expression_type(value)) {
                    __THROW_ERROR(
                        _generator->fpath, 
                        _generator->fdata, 
                        value->position, 
                        "variable value must be an expression"
                    );
                }
                if (scope_has(_scope, name->str0, true)) {
                    __THROW_ERROR(
                        _generator->fpath, 
                        _generator->fdata, 
                        name->position, 
                        "symbol %s is already defined", name->str0
                    );
                    return;
                }
                // Generate value code
                if (value) {
                    generator_expression(_generator, value);
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
                if (!cond || !tvalue) PD(!cond ? "if statement must have a condition, but received NULL" : "if statement must have a true value, but received NULL");
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    cond->position, 
                    "condition must be an expression"
                );
            }
            if (generator_is_constant_node(cond) || !generator_is_logical_expression(cond)) {
                if (generator_is_constant_node(cond)) {
                    FOLD_CONSTANT_EXPRESSION(cond);
                } else {
                    generator_expression(_generator, cond);
                }
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
                generator_set_4bytes(_generator, jump_endif_from_true , _generator->bsize - jump_endif_from_true );
                generator_set_4bytes(_generator, jump_endif_from_false, _generator->bsize - jump_endif_from_false);
                free(cond);
                free(_statement);
                break;
            } else {
                ast_node_t* cond_l = cond->ast0;
                ast_node_t* cond_r = cond->ast1;
                if (!cond_l || !cond_r) {
                    PD(!cond_l ? "logical expression must have a left operand, but received NULL" :
                       !cond_r ? "logical expression must have a right operand, but received NULL" :
                       "logical expression must have two operands, but received NULL");
                }
                bool is_logical_and = cond->type == AstLogicalAnd;
                if (is_logical_and) {
                    generator_expression(_generator, cond_l);
                    generator_emit_byte(_generator, OPCODE_POP_JUMP_IF_FALSE);
                    int jump_start_l = _generator->bsize;
                    generator_allocate_nbytes(_generator, 4);
                    // If left is true, then evaluate right
                    generator_expression(_generator, cond_r);
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
                    free(cond_l);
                    free(cond_r);
                    free(_statement);
                } else {
                    generator_expression(_generator, cond_l);
                    generator_emit_byte(_generator, OPCODE_POP_JUMP_IF_TRUE);
                    int jump_start_l = _generator->bsize;
                    generator_allocate_nbytes(_generator, 4);
                    // If left is false, then evaluate right
                    generator_expression(_generator, cond_r);
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
                    free(cond_l);
                    free(cond_r);
                    free(_statement);
                }
            }
            break;
        }
        case AstReturn: {
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
            for (current = _scope; current->type != ScopeTypeFunction; current = current->parent) {}
            current->is_returned = true;
            // Generate return value and return opcode
            ast_node_t* expr = _statement->ast0;
            if (expr != NULL) {
                if (!generator_is_expression_type(expr)) {
                    __THROW_ERROR(
                        _generator->fpath, 
                        _generator->fdata, 
                        expr->position, 
                        "return value must be an expression"
                    );
                }
                generator_expression(_generator, expr);
                free(expr);
            } else {
                generator_emit_byte(_generator, OPCODE_LOAD_NULL);
            }
            generator_emit_byte(_generator, OPCODE_RETURN);
            free(_statement);
            break;
        }
        case AstStatementExpression: {
            if (_statement->ast0 == NULL) {
                PD("expression statement must have an expression, but received NULL");
            }
            generator_expression(_generator, _statement->ast0);
            generator_emit_byte(_generator, OPCODE_POPTOP);
            free(_statement);
            break;
        }
        default:
            PD("unsupported statement type %d, but received %d", _statement->type, _statement->type);
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
        if (children[i] != NULL) generator_statement(_generator, scope, children[i]);
    }
    free(children);
    free(_program);
    scope_free(scope);
    // write the bytecode to the file
    generator_emit_byte(_generator, OPCODE_LOAD_NULL);
    generator_emit_byte(_generator, OPCODE_RETURN);
    // Set the bytecode size
    generator_set_8bytes(_generator, 8, _generator->bsize);
}

// -----------------------------

DLLEXPORT generator_t* generator_new(char* _fpath, char* _fdata) {
    generator_t* generator = (generator_t*) malloc(sizeof(generator_t));
    ASSERTNULL(generator, ERROR_ALLOCATING_GENERATOR);
    generator->fpath = string_allocate(_fpath);
    generator->fdata = string_allocate(_fdata);
    generator->fsize = strlen(_fdata);
    generator->bsize = 0;
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