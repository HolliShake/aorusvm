#include "api/core/generator.h"
#include "api/core/vm.h"
#include "code.h"
#include "error.h"
#include "eval.h"
#include "func.h"
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
    size_t   codelen;
} generator_t;


#define EXPECT(_size) if (_generator->bsize != _size) PD("INCORRECT ALLOCATION: BSIZE: %zu != %d", _generator->bsize, _size);

#define generator_is_statement_type(type) (!generator_is_expression_type(type))

INTERNAL bool generator_is_expression_type(ast_node_t* _expression) {
    switch (_expression->type) {
        case AstName:
        case AstInt:
        case AstLong:
        case AstFloat:
        case AstDouble:
        case AstString:
        case AstBoolean:
        case AstNull:
        case AstThis:
        case AstSuper:
        case AstArray:
        case AstObject:
        case AstFunctionExpression:
        case AstMemberAccess:
        case AstIndex:
        case AstCall:
        case AstPostfixPlusPlus:
        case AstPostfixMinusMinus:
        case AstUnaryPlusPlus:
        case AstUnaryMinusMinus:
        case AstUnaryPlus:
        case AstUnaryMinus:
        case AstUnaryNot:
        case AstUnaryBitnot:
        case AstUnarySpread:
        case AstNew:
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
        case AstAssign:
        case AstRange:
        case AstCatch:
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
        case AstString:
        case AstBoolean:
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

INTERNAL void resize(code_t* _code, size_t _size) {
    _code->bytecode = (uint8_t*) realloc(_code->bytecode, sizeof(uint8_t) * (_code->size + _size));
    ASSERTNULL(_code->bytecode, "failed to allocate memory for bytecode");
}

INTERNAL void emit(code_t* _code, uint8_t _value) {
    resize(_code, 1);
    _code->bytecode[_code->size++] = _value;
}

INTERNAL void emit_int(code_t* _code, int _value) {
    resize(_code, 4);
    for (size_t i = 0; i < 4; i++) {
        _code->bytecode[_code->size++] = (_value >> (i * 8)) & 0xFF;
    }
}

INTERNAL void emit_double(code_t* _code, double _value) {
    union double_bytes_t {
        double f64;
        uint8_t bytes[8];
    } double_bytes = {
        .f64 = _value,
    };
    resize(_code, 8);
    for (size_t i = 0; i < 8; i++) {
        _code->bytecode[_code->size++] = double_bytes.bytes[i];
    }
}

INTERNAL void emit_memory(code_t* _code, void* _value) {
    #if defined(IS_64BIT)
    uintptr_t address = (uintptr_t) _value;
    #else
    uintptr_t address = (uintptr_t) _value;
    #endif
    resize(_code, 8);
    for (size_t i = 0; i < 8; i++) {
        _code->bytecode[_code->size++] = (uint8_t)((address >> (i * 8)) & 0xFF);
    }
}

INTERNAL void emit_string(code_t* _code, char* _value) {
    size_t len0 = strlen(_value);
    size_t len1 = len0 + 1;
    resize(_code, len1);
    for (size_t i = 0; i < len0; i++) {
        _code->bytecode[_code->size++] = _value[i];
    }
    _code->bytecode[_code->size++] = 0;
}

INTERNAL int emit_jump(code_t* _code, opcode_t _opcode) {
    emit(_code, _opcode);
    int start = (int) _code->size;
    if (start != _code->size) PD("address overflow: %d != %zu", start, _code->size);
    emit_int(_code, 0);
    return start;
}

INTERNAL void emit_jumpto(code_t* _code, opcode_t _opcode, int _start) {
    emit(_code, _opcode);
    emit_int(_code, _start);
}

INTERNAL int here(code_t* _code) {
    return _code->size;
}

INTERNAL void label(code_t* _code, int _start) {
    for (size_t i = 0; i < 4; i++) {
        _code->bytecode[_start + i] = (_code->size >> (i * 8)) & 0xFF;
    }
}

#define FOLD_CONSTANT_EXPRESSION(expression) { \
    eval_result_t result = eval_eval(expression); \
    switch (result.type) { \
        case EvalInt: \
            emit(_code, OPCODE_LOAD_INT); \
            emit_int(_code, result.value.i32); \
            break; \
        case EvalDouble: \
            emit(_code, OPCODE_LOAD_DOUBLE); \
            emit_double(_code, result.value.f64); \
            break; \
        case EvalBoolean: \
            emit(_code, OPCODE_LOAD_BOOL); \
            emit(_code, result.value.i32 == 1); \
            break; \
        case EvalString: \
            emit(_code, OPCODE_LOAD_STRING); \
            emit_string(_code, (char*) result.value.ptr); \
            break; \
        case EvalNull: \
            emit(_code, OPCODE_LOAD_NULL); \
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

INTERNAL void generator_expression(generator_t* _generator, code_t* _code, scope_t* _scope, ast_node_t* _expression);
INTERNAL void generator_statement(generator_t* _generator, code_t* _code, scope_t* _scope, ast_node_t* _statement);

INTERNAL void generator_assignment(generator_t* _generator, code_t* _code, scope_t* _scope, ast_node_t* _expression) {
    if (_expression == NULL) {
        __THROW_ERROR(
            _generator->fpath,
            _generator->fdata,
            _expression->position,
            "assignment expression must have an expression, but received NULL"
        );
    }
    if (!generator_is_expression_type(_expression)) {
        __THROW_ERROR(
            _generator->fpath,
            _generator->fdata,
            _expression->position,
            "assignment expression must be an expression, but received %d", _expression->type
        );
    }
    ast_node_t* lhs = _expression->ast0;
    ast_node_t* rhs = _expression->ast1;
    switch (lhs->type) {
        case AstName:
            generator_expression(_generator, _code, _scope, rhs);
            emit(_code, OPCODE_SET_NAME);
            emit_string(_code, lhs->str0);
            break;
        case AstMemberAccess:
            generator_expression(_generator, _code, _scope, rhs); // value
            generator_expression(_generator, _code, _scope, lhs->ast0); //object
            emit(_code, OPCODE_SET_PROPERTY);
            emit_string(_code, lhs->ast1->str0);
            break;
        default:
            __THROW_ERROR(
                _generator->fpath,
                _generator->fdata,
                _expression->position,
                "assignment expression must be a name or member access, but received %d", _expression->type
            );
    }
}

INTERNAL void generator_assignment0(generator_t* _generator, code_t* _code, scope_t* _scope, ast_node_t* _expression) {
    if (_expression == NULL) {
        __THROW_ERROR(
            _generator->fpath,
            _generator->fdata,
            _expression->position,
            "assignment expression must have an expression, but received NULL"
        );
    }
    if (!generator_is_expression_type(_expression)) {
        __THROW_ERROR(
            _generator->fpath,
            _generator->fdata,
            _expression->position,
            "assignment expression must be an expression, but received %d", _expression->type
        );
    }
    switch (_expression->type) {
        case AstName:
            emit(_code, OPCODE_LOAD_NAME);
            emit_string(_code, _expression->str0);
            if (scope_is_function(_scope) && !scope_function_has(_scope, _expression->str0)) {
                scope_save_capture(_scope, _expression->str0);
            }
            emit(_code, OPCODE_DUPTOP);
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

INTERNAL void generator_assignment1(generator_t* _generator, code_t* _code, scope_t* _scope, ast_node_t* _expression, bool _is_postfix) {
    if (_expression == NULL) {
        __THROW_ERROR(
            _generator->fpath,
            _generator->fdata,
            _expression->position,
            "assignment expression must have an expression, but received NULL"
        );
    }
    if (!generator_is_expression_type(_expression)) {
        __THROW_ERROR(
            _generator->fpath, 
            _generator->fdata, 
            _expression->position, 
            "assignment expression must be an expression, but received %d", _expression->type
        );
    }
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
            if (_is_postfix) emit(_code, OPCODE_ROT3);
            emit(_code, OPCODE_SET_NAME);
            emit_string(
                _code, 
                _expression->str0
            );
            emit(_code, OPCODE_POPTOP);
            break;
        case AstMemberAccess:
            generator_expression(_generator, _code, _scope, _expression->ast0);
            emit(_code, OPCODE_SET_PROPERTY);
            emit_string(_code, _expression->ast1->str0);
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

INTERNAL void generator_function(generator_t* _generator, code_t* _code, scope_t* _scope, ast_node_t* _expression) {
    ast_node_list_t params = _expression->array0;
    ast_node_list_t body   = _expression->array1;
    // Validate name
    if (params == NULL) {
        __THROW_ERROR(
            _generator->fpath, 
            _generator->fdata, 
            _expression->position, 
            "function must have parameters, but received NULL"
        );
    }
    if (body == NULL) {
        __THROW_ERROR(
            _generator->fpath, 
            _generator->fdata, 
            _expression->position, 
            "function must have a body, but received NULL"
        );
    }
    size_t param_count;
    for (param_count = 0; params[param_count] != NULL; param_count++);
    code_t* _func = code_new_function(
        string_allocate(_generator->fpath),
        string_allocate("function"),
        true,// Function expression is always scoped
        false,
        param_count,
        (uint8_t*) malloc(sizeof(uint8_t)),
        0
    );
    // Make function
    emit(_code, OPCODE_SETUP_FUNCTION);
    emit(_code, OPCODE_BEGIN_FUNCTION);
    emit_memory(_code, _func);
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
        if (scope_function_has(local_scope, param->str0)) {
            __THROW_ERROR(
                _generator->fpath, 
                _generator->fdata, 
                param->position, 
                "function parameter %s is already defined", param->str0
            );
        }
        // Save into symbol table
        scope_value_t symbol = {
            .name      = param->str0,
            .is_const  = false,
            .is_global = true,
            .position  = param->position
        };
        scope_put(local_scope, param->str0, symbol);
        // Emit the store name opcode
        emit(_func, OPCODE_STORE_NAME);
        emit_string(_func, param->str0);
    }
    // Compile body
    bool has_visible_return = false;
    for (size_t i = 0; body[i] != NULL; i++) {
        ast_node_t* statement = body[i];
        if (body[i]->type == AstReturnStatement && has_visible_return) {
            break;
        }
        if (body[i]->type == AstReturnStatement) has_visible_return = true;
        generator_statement(_generator, _func, local_scope, statement); // statement
    }
    if (!has_visible_return) {
        // Emit the return opcode
        emit(_func, OPCODE_LOAD_NULL);
        emit(_func, OPCODE_RETURN);
    }
    // Save captures
    if (function_scope->capture_count > 0) {
        // Emit opcode save captures
        emit(_code, OPCODE_SAVE_CAPTURES);
        emit_int(_code, function_scope->capture_count);
        for (size_t i = 0; i < function_scope->capture_count; i++) {
            emit_string(_code, function_scope->captures[i]);
        }
    }
    // Free the function scope
    scope_free(local_scope);
    scope_free(function_scope);
}

INTERNAL void generator_expression(generator_t* _generator, code_t* _code, scope_t* _scope, ast_node_t* _expression) {
    if (_expression == NULL) {
        __THROW_ERROR(
            _generator->fpath,
            _generator->fdata,
            _expression->position,
            "expression must have an expression, but received NULL"
        );
    }
    if (!generator_is_expression_type(_expression)) {
        __THROW_ERROR(
            _generator->fpath, 
            _generator->fdata, 
            _expression->position, 
            "expression must be a valid expression type"
        );
    }
    switch (_expression->type) {
        case AstName:
            emit(_code, OPCODE_LOAD_NAME);
            emit_string(_code, _expression->str0);
            if (scope_is_function(_scope) && !scope_function_has(_scope, _expression->str0)) {
                scope_save_capture(_scope, _expression->str0);
            }
            break;
        case AstInt:
        case AstFloat:
            emit(_code, OPCODE_LOAD_INT);
            emit_int(_code, (int) _expression->value.i32);
            break;
        case AstLong:
        case AstDouble:
            emit(_code, OPCODE_LOAD_DOUBLE);
            emit_double(_code, (double) _expression->value.i64);
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
            emit(_code, OPCODE_LOAD_STRING);
            emit_string(_code, _expression->str0);
            break;
        case AstBoolean:
            emit(_code, OPCODE_LOAD_BOOL);
            emit(_code, (uint8_t) (_expression->value.i32 == 1));
            break;
        case AstNull:
            emit(_code, OPCODE_LOAD_NULL);
            break;
        case AstThis:
            if ((!scope_is_class(_scope) && !scope_is_object(_scope, true)) && !scope_is_function(_scope)) {
                __THROW_ERROR(
                    _generator->fpath,
                    _generator->fdata,
                    _expression->position,
                    "this can only be used in a class"
                );
            }
            emit(_code, OPCODE_LOAD_THIS);
            break;
        case AstSuper:
            if (!scope_is_class(_scope)) {
                __THROW_ERROR(
                    _generator->fpath,
                    _generator->fdata,
                    _expression->position,
                    "super can only be used in a class"
                );
            }
            emit(_code, OPCODE_LOAD_SUPER);
            break;
        case AstArray: {
            scope_t* array_scope = scope_new(_scope, ScopeTypeArray);
            ast_node_list_t elements = _expression->array0;
            // Count elements first
            bool has_spread = false;
            size_t count = 0;
            for (size_t i = 0; elements[i] != NULL; i++) {
                if (elements[i]->type == AstUnarySpread) {
                    has_spread = true;
                }
                count++;
            }
            // Generate elements right to left for reversed array
            size_t i;
            if (!has_spread) {
                for (i = count; i > 0; i--) {
                    ast_node_t* element = elements[i-1];
                    generator_expression(_generator, _code, array_scope, element);
                }
                emit(_code, OPCODE_LOAD_ARRAY);
                emit_int(_code, count);
            } else {
                // Dynamic array with possible spread
                emit(_code, OPCODE_LOAD_ARRAY);
                emit_int(_code, 0); // Start from empty array

                for (i = 0; i < count; i++) {
                    ast_node_t* element = elements[i];

                    if (element->type == AstUnarySpread) {
                        generator_expression(_generator, _code, array_scope, element->ast0);
                        emit(_code, OPCODE_EXTEND_ARRAY);
                    } else {
                        generator_expression(_generator, _code, array_scope, element);
                        emit(_code, OPCODE_APPEND_ARRAY);
                    }
                }
            }
            scope_free(array_scope);
            break;
        }
        case AstObjectProperty: {
            ast_node_t* key = _expression->ast0;
            ast_node_t* value = _expression->ast1;
            if (!scope_is_object(_scope, false)) {
                __THROW_ERROR(
                    _generator->fpath,
                    _generator->fdata,
                    _expression->position,
                    "object property can only be used in an object"
                );
            }
            if (key == NULL) {
                __THROW_ERROR(
                    _generator->fpath,
                    _generator->fdata,
                    _expression->position,
                    "object property must have a key"
                );
            }
            if (value == NULL) {
                __THROW_ERROR(
                    _generator->fpath,
                    _generator->fdata,
                    _expression->position,
                    "object property must have a value"
                );
            }
            // Or ignore it!!!
            generator_expression(_generator, _code, _scope, value); // value
            generator_expression(_generator, _code, _scope, key); // key
            emit(_code, OPCODE_PUT_OBJECT);
            break;
        }
        case AstObject: {
            scope_t* object_scope = scope_new(_scope, ScopeTypeObject);
            ast_node_list_t properties = _expression->array0;
            // Count elements first
            bool has_spread = false;
            size_t count = 0;
            for (size_t i = 0; properties[i] != NULL; i++) {
                if (properties[i]->type == AstUnarySpread) {
                    has_spread = true;
                }
                count++;
            }
            // Generate elements right to left for reversed array
            size_t i;
            if (!has_spread) {
                for (i = count; i > 0; i--) {
                    ast_node_t* property = properties[i-1];
                    if (property->type != AstObjectProperty) {
                        __THROW_ERROR(
                            _generator->fpath,
                            _generator->fdata,
                            property->position,
                            "object property expected"
                        );
                    }
                    generator_expression(_generator, _code, object_scope, property->ast1); // value
                    generator_expression(_generator, _code, object_scope, property->ast0); // key
                }
                emit(_code, OPCODE_LOAD_OBJECT);
                emit_int(_code, count);
            } else {
                // Dynamic object with possible spread
                emit(_code, OPCODE_LOAD_OBJECT);
                emit_int(_code, 0); // Start from empty object

                for (i = 0; i < count; i++) {
                    ast_node_t* property = properties[i];

                    if (property->type == AstUnarySpread) {
                        generator_expression(_generator, _code, object_scope, property->ast0);
                        emit(_code, OPCODE_EXTEND_OBJECT);
                    } else {
                        if (property->type != AstObjectProperty) {
                            __THROW_ERROR(
                                _generator->fpath,
                                _generator->fdata,
                                property->position,
                                "object property expected"
                            );
                        }
                        generator_expression(_generator, _code, object_scope, property->ast1); // value
                        generator_expression(_generator, _code, object_scope, property->ast0); // key
                        emit(_code, OPCODE_PUT_OBJECT);
                    }
                }
            }
            scope_free(object_scope);
            break;
        }
        case AstFunctionExpression: {
            generator_function(_generator, _code, _scope, _expression);
            break;
        }
        case AstNew: {
            ast_node_t* expression = _expression->ast0;
            ast_node_list_t arguments = _expression->array0;
            if (expression == NULL) {
                __THROW_ERROR(
                    _generator->fpath,
                    _generator->fdata,
                    _expression->position,
                    "new expression must have a value, but received NULL"
                );
            }
            if (arguments == NULL) {
                __THROW_ERROR(
                    _generator->fpath,
                    _generator->fdata,
                    _expression->position,
                    "new expression must have arguments, but received NULL"
                );
            }
            int param_count = 0;
            for (param_count = 0; arguments[param_count] != NULL; param_count++);
            for (int i = param_count - 1; i >= 0; i--) {
                generator_expression(_generator, _code, _scope, arguments[i]);
            }
            generator_expression(_generator, _code, _scope, expression);
            emit(_code, OPCODE_CALL_CONSTRUCTOR);
            emit_int(_code, param_count);
            break;
        }
        case AstMemberAccess: {
            ast_node_t* obj    = _expression->ast0;
            ast_node_t* member = _expression->ast1;
            if (obj == NULL) {
                __THROW_ERROR(
                    _generator->fpath,
                    _generator->fdata,
                    _expression->position,
                    "member access expression requires an object, but received NULL"
                );
            }
            if (member == NULL) {
                __THROW_ERROR(
                    _generator->fpath,
                    _generator->fdata,
                    _expression->position,
                    "member access expression requires a member, but received NULL"
                );
            }
            if (member->type != AstName) {
                __THROW_ERROR(
                    _generator->fpath,
                    _generator->fdata,
                    _expression->position,
                    "member access expression requires a member, but received %d", member->type
                );
            }
            generator_expression(_generator, _code, _scope, obj);
            emit(_code, OPCODE_GET_PROPERTY);
            emit_string(_code, member->str0);
            break;
        }
        case AstIndex: {
            ast_node_t* obj = _expression->ast0;
            ast_node_t* index = _expression->ast1;
            if (obj == NULL) {
                __THROW_ERROR(
                    _generator->fpath,
                    _generator->fdata,
                    _expression->position,
                    "index expression requires an object, but received NULL"
                );
            }
            if (index == NULL) {
                __THROW_ERROR(
                    _generator->fpath,
                    _generator->fdata,
                    _expression->position,
                    "index expression requires an index, but received NULL"
                );
            }
            generator_expression(_generator, _code, _scope, obj);
            generator_expression(_generator, _code, _scope, index);
            emit(_code, OPCODE_INDEX);
            break;
        }
        case AstCall: {
            ast_node_t* function      = _expression->ast0;
            ast_node_list_t arguments = _expression->array0;
            if (function == NULL) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "call expression must have a function, but received NULL"
                );
            }
            if (arguments == NULL) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "call expression must have arguments, but received NULL"
                );
            }
            int param_count = 0;
            // Count arguments first
            for (param_count = 0; arguments[param_count] != NULL; param_count++);
            // Generate arguments right to left for stack
            for (int i = param_count - 1; i >= 0; i--) {
                ast_node_t* argument = arguments[i];
                generator_expression(_generator, _code, _scope, argument);
            }
            if (function->type == AstMemberAccess) {
                ast_node_t* obj = function->ast0;
                ast_node_t* member = function->ast1;
                if (obj == NULL) {
                    __THROW_ERROR(
                        _generator->fpath,
                        _generator->fdata,
                        _expression->position,
                        "call expression must have a function, but received NULL"
                    );
                }
                if (member == NULL) {
                    __THROW_ERROR(
                        _generator->fpath,
                        _generator->fdata,
                        _expression->position,
                        "call expression must have a function, but received NULL"
                    );
                }
                if (member->type != AstName) {
                    __THROW_ERROR(
                        _generator->fpath,
                        _generator->fdata,
                        _expression->position,
                        "call expression must have a function, but received %d", member->type
                    );
                }
                generator_expression(_generator, _code, _scope, obj);
                emit(_code, OPCODE_CALL_METHOD);
                emit_string(_code, member->str0);
                emit_int(_code, param_count);
            } else {
                generator_expression(_generator, _code, _scope, function);
                emit(_code, OPCODE_CALL);
                emit_int(_code, param_count);
            }
            break;
        }
        case AstPostfixPlusPlus: {
            ast_node_t* expression = _expression->ast0;
            if (expression == NULL) {
                __THROW_ERROR(
                    _generator->fpath,
                    _generator->fdata,
                    _expression->position,
                    "unary expression must have an expression, but received NULL"
                );
            }
            generator_assignment0(_generator, _code, _scope, expression);
            emit(_code, OPCODE_INCREMENT);
            generator_assignment1(_generator, _code, _scope, expression, true);
            break;
        }
        case AstUnaryPlusPlus: {
            ast_node_t* expression = _expression->ast0;
            if (expression == NULL) {
                __THROW_ERROR(
                    _generator->fpath,
                    _generator->fdata,
                    _expression->position,
                    "unary expression must have an expression, but received NULL"
                );
            }
            generator_assignment0(_generator, _code, _scope, expression);
            emit(_code, OPCODE_INCREMENT);
            generator_assignment1(_generator, _code, _scope, expression, false);
            break;
        }
        case AstUnaryMinusMinus: {
            ast_node_t* expression = _expression->ast0;
            if (expression == NULL) {
                __THROW_ERROR(
                    _generator->fpath,
                    _generator->fdata,
                    _expression->position,
                    "unary expression must have an expression, but received NULL"
                );
            }
            generator_assignment0(_generator, _code, _scope, expression);
            emit(_code, OPCODE_DECREMENT);
            generator_assignment1(_generator, _code, _scope, expression, false);
            break;
        }
        case AstUnaryPlus: {
            ast_node_t* expression = _expression->ast0;
            if (expression == NULL) {
                __THROW_ERROR(
                    _generator->fpath,
                    _generator->fdata,
                    _expression->position,
                    "unary expression must have an expression, but received NULL"
                );
            }
            generator_expression(_generator, _code, _scope, expression);
            emit(_code, OPCODE_UNARY_PLUS);
            break;
        }
        case AstUnaryMinus: {
            ast_node_t* expression = _expression->ast0;
            if (expression == NULL) {
                __THROW_ERROR(
                    _generator->fpath,
                    _generator->fdata,
                    _expression->position,
                    "unary expression must have an expression, but received NULL"
                );
            }
            generator_expression(_generator, _code, _scope, expression);
            emit(_code, OPCODE_UNARY_MINUS);
            break;
        }
        case AstUnaryNot: {
            ast_node_t* expression = _expression->ast0;
            if (expression == NULL) {
                __THROW_ERROR(
                    _generator->fpath,
                    _generator->fdata,
                    _expression->position,
                    "unary expression must have an expression, but received NULL"
                );
            }
            generator_expression(_generator, _code, _scope, expression);
            emit(_code, OPCODE_NOT);
            break;
        }
        case AstUnaryBitnot: {
            ast_node_t* expression = _expression->ast0;
            if (expression == NULL) {
                __THROW_ERROR(
                    _generator->fpath,
                    _generator->fdata,
                    _expression->position,
                    "unary expression must have an expression, but received NULL"
                );
            }
            generator_expression(_generator, _code, _scope, expression);
            emit(_code, OPCODE_BITWISE_NOT);
            break;
        }
        case AstUnarySpread: {
            ast_node_t* expression = _expression->ast0;
            if (!scope_is_array(_scope) && !scope_is_object(_scope, false)) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _expression->position, 
                    "spread operator can only be used in an array or object"
                );
            }
            if (expression == NULL) {
                __THROW_ERROR(
                    _generator->fpath,
                    _generator->fdata,
                    _expression->position,
                    "spread operator must have an expression, but received NULL"
                );
            }
            // Or ignore it!!!
            generator_expression(_generator, _code, _scope, expression);
            emit(_code, OPCODE_EXTEND_ARRAY);
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
            if (generator_is_constant_node(_expression)) {
                FOLD_CONSTANT_EXPRESSION(_expression);
                return;
            }
            generator_expression(
                _generator, 
                _code,
                _scope,
                _expression->ast0
            );
            generator_expression(
                _generator, 
                _code,
                _scope,
                _expression->ast1
            );
            emit(_code, OPCODE_MUL);
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
            if (generator_is_constant_node(_expression)) {
                FOLD_CONSTANT_EXPRESSION(_expression);
                return;
            }
            generator_expression(
                _generator, 
                _code,
                _scope,
                _expression->ast0
            );
            generator_expression(
                _generator, 
                _code,
                _scope,
                _expression->ast1
            );
            emit(_code, OPCODE_DIV);
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
                _code,
                _scope,
                _expression->ast0
            );
            generator_expression(
                _generator, 
                _code,
                _scope,
                _expression->ast1
            );
            emit(_code, OPCODE_MOD);
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
            if (generator_is_constant_node(_expression)) {
                FOLD_CONSTANT_EXPRESSION(_expression);
                return;
            }
            generator_expression(
                _generator, 
                _code,
                _scope,
                _expression->ast0
            );
            generator_expression(
                _generator, 
                _code,
                _scope,
                _expression->ast1
            );
            emit(_code, OPCODE_ADD);
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
            if (generator_is_constant_node(_expression)) {
                FOLD_CONSTANT_EXPRESSION(_expression);
                return;
            }
            generator_expression(
                _generator, 
                _code,
                _scope,
                _expression->ast0
            );
            generator_expression(
                _generator, 
                _code,
                _scope,
                _expression->ast1
            );
            emit(_code, OPCODE_SUB);
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
            if (generator_is_constant_node(_expression)) {
                FOLD_CONSTANT_EXPRESSION(_expression);
                return;
            }
            generator_expression(
                _generator, 
                _code,
                _scope,
                _expression->ast0
            );
            generator_expression(
                _generator, 
                _code,
                _scope,
                _expression->ast1
            );
            emit(_code, OPCODE_SHL);
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
            if (generator_is_constant_node(_expression)) {
                FOLD_CONSTANT_EXPRESSION(_expression);
                return;
            }
            generator_expression(
                _generator, 
                _code,
                _scope,
                _expression->ast0
            );
            generator_expression(
                _generator, 
                _code,
                _scope,
                _expression->ast1
            );
            emit(_code, OPCODE_SHR);
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
            if (generator_is_constant_node(_expression)) {
                FOLD_CONSTANT_EXPRESSION(_expression);
                return;
            }
            generator_expression(
                _generator, 
                _code,
                _scope,
                _expression->ast0
            );
            generator_expression(
                _generator, 
                _code,
                _scope,
                _expression->ast1
            );
            emit(_code, OPCODE_CMP_LT);
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
                _code,
                _scope,
                _expression->ast0
            );
            generator_expression(
                _generator, 
                _code,
                _scope,
                _expression->ast1
            );
            emit(_code, OPCODE_CMP_LTE);
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
                _code,
                _scope,
                _expression->ast0
            );
            generator_expression(
                _generator, 
                _code,
                _scope,
                _expression->ast1
            );
            emit(_code, OPCODE_CMP_GT);
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
                _code,
                _scope,
                _expression->ast0
            );
            generator_expression(
                _generator, 
                _code,
                _scope,
                _expression->ast1
            );
            emit(_code, OPCODE_CMP_GTE);
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
                _code,
                _scope,
                _expression->ast0
            );
            generator_expression(
                _generator, 
                _code,
                _scope,
                _expression->ast1
            );
            emit(_code, OPCODE_CMP_EQ);
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
                _code,
                _scope,
                _expression->ast0
            );
            generator_expression(
                _generator, 
                _code,
                _scope,
                _expression->ast1
            );
            emit(_code, OPCODE_CMP_NE);
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
            if (generator_is_constant_node(_expression)) {
                FOLD_CONSTANT_EXPRESSION(_expression);
                return;
            }
            generator_expression(
                _generator, 
                _code,
                _scope,
                _expression->ast0
            );
            int jump_start = emit_jump(_code, OPCODE_JUMP_IF_FALSE_OR_POP);
            generator_expression(
                _generator, 
                _code,
                _scope,
                _expression->ast1
            );
            label(_code, jump_start);
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
            if (generator_is_constant_node(_expression)) {
                FOLD_CONSTANT_EXPRESSION(_expression);
                return;
            }
            generator_expression(
                _generator, 
                _code,
                _scope,
                _expression->ast0
            );
            int jump_start = emit_jump(_code, OPCODE_JUMP_IF_TRUE_OR_POP);
            generator_expression(
                _generator, 
                _code,
                _scope,
                _expression->ast1
            );
            label(_code, jump_start);
            break;
        }
        case AstAssign: {
            generator_assignment(_generator, _code, _scope, _expression);
            break;
        }
        case AstRange: {
            ast_node_t* lhs = _expression->ast0;
            ast_node_t* rhs = _expression->ast1;
            if (lhs == NULL) {
                __THROW_ERROR(
                    _generator->fpath,
                    _generator->fdata,
                    _expression->position,
                    "range expression requires a left hand side, but received NULL"
                );
            }
            if (rhs == NULL) {
                __THROW_ERROR(
                    _generator->fpath,
                    _generator->fdata,
                    _expression->position,
                    "range expression requires a right hand side, but received NULL"
                );
            }
            generator_expression(_generator, _code, _scope, rhs);
            generator_expression(_generator, _code, _scope, lhs);
            emit(_code, OPCODE_RANGE);
            break;
        }
        case AstCatch: {
            ast_node_t* error       = _expression->ast0;
            ast_node_t* placeholder = _expression->ast1;
            ast_node_list_t body    = _expression->array0;
            if (error == NULL) {
                __THROW_ERROR(
                    _generator->fpath,
                    _generator->fdata,
                    _expression->position,
                    "catch error expected"
                );
            }
            if (placeholder == NULL) {
                __THROW_ERROR(
                    _generator->fpath,
                    _generator->fdata,
                    _expression->position,
                    "catch placeholder expected"
                );
            }
            if (body == NULL) {
                __THROW_ERROR(
                    _generator->fpath,
                    _generator->fdata,
                    _expression->position,
                    "catch body expected"
                );
            }
            if (placeholder->type != AstName) {
                __THROW_ERROR(
                    _generator->fpath,
                    _generator->fdata,
                    _expression->position,
                    "catch placeholder must be a name"
                );
            }
            // Generate error code
            generator_expression(_generator, _code, _scope, error);
            int end = emit_jump(_code, OPCODE_JUMP_IF_NOT_ERROR);
            
            code_t* _catch = code_new_block(
                string_allocate(_generator->fpath),
                string_allocate("catch"),
                (uint8_t*) malloc(sizeof(uint8_t)),
                0
            );
            // Setup catch block
            scope_t* catch_scope = scope_new(_scope, ScopeTypeCatch);
            scope_t* local_scope = scope_new(catch_scope, ScopeTypeLocal);
            // Setup block and reserve space for metadata
            emit(_code, OPCODE_SETUP_CATCH_BLOCK);
            emit_memory(_code, _catch);
            
            // Store placeholder
            emit(_catch, OPCODE_STORE_NAME);
            emit_string(_catch, placeholder->str0);
            // Body
            bool has_visible_return = false;
            for (size_t i = 0; body[i] != NULL; i++) {
                if (body[i]->type == AstReturnStatement && has_visible_return) {
                    break;
                }
                if (body[i]->type == AstReturnStatement) has_visible_return = true;
                generator_statement(_generator, _catch, catch_scope, body[i]);
            }
            // Return placeholder
            if (!has_visible_return) {
                emit(_catch, OPCODE_LOAD_NULL);
                emit(_catch, OPCODE_RETURN);
            }
            // Jump here if there is no error
            label(_code, end);
            // Free the scopes
            scope_free(local_scope);
            scope_free(catch_scope);
            break;
        }
        default:
            PD("unsupported expression type %d, but received %d", _expression->type, _expression->type);
    }
}

//==================================

INTERNAL void generator_statement(generator_t* _generator, code_t* _code, scope_t* _scope, ast_node_t* _statement) {
    if (_statement == NULL) {
        __THROW_ERROR(
            _generator->fpath,
            _generator->fdata,
            _statement->position,
            "statement must have a statement, but received NULL"
        );
    }
    if (!generator_is_statement_type(_statement)) {
        __THROW_ERROR(
            _generator->fpath, 
            _generator->fdata, 
            _statement->position, 
            "statement must be a valid statement type"
        );
    }
    switch (_statement->type) {
        case AstVarStatement:
        case AstConstStatement:
        case AstLocalStatement: {
            ast_node_list_t names  = _statement->array0;
            ast_node_list_t values = _statement->array1;

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
                    generator_expression(_generator, _code, _scope, value);
                } else {
                    emit(_code, OPCODE_LOAD_NULL);
                }

                // Store value
                emit(_code, OPCODE_STORE_NAME);
                emit_string(_code, name->str0);

                // Add to scope
                scope_value_t symbol = {
                    .name      = name->str0,
                    .is_const  = (_statement->type == AstConstStatement),
                    .is_global = scope_is_global(_scope),
                    .position  = name->position
                };
                scope_put(_scope, name->str0, symbol);
            }
            break;
        }
        case AstIfStatement: {
            ast_node_t* cond   = _statement->ast0;
            ast_node_t* tvalue = _statement->ast1;
            ast_node_t* fvalue = _statement->ast2;
            if (!cond || !tvalue) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _statement->position, 
                    "if statement must have a condition, a true value, and a false value"
                );
            }
            if (generator_is_constant_node(cond) || !generator_is_logical_expression(cond)) {
                // Condition
                generator_expression(_generator, _code, _scope, cond);
                // Jump if false
                int to_else = emit_jump(_code, OPCODE_POP_JUMP_IF_FALSE);
                // true
                generator_statement(_generator, _code, _scope, tvalue);
                // Jump to endif after true
                int to_end = emit_jump(_code, OPCODE_JUMP_FORWARD);
                // false?
                label(_code, to_else);
                if (fvalue != NULL) {
                    generator_statement(_generator, _code, _scope, fvalue);
                }
                label(_code, to_end);
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
                bool is_logical_and = cond->type == AstLogicalAnd;
                if (is_logical_and) {
                    generator_expression(_generator, _code, _scope, cond_l);
                    int jump_start_l = emit_jump(_code, OPCODE_POP_JUMP_IF_FALSE);
                    // If left is true, then evaluate right
                    generator_expression(_generator, _code, _scope, cond_r);
                    int jump_start_r = emit_jump(_code, OPCODE_POP_JUMP_IF_FALSE);
                    // true
                    generator_statement(_generator, _code, _scope, tvalue);
                    // Jump to endif from true
                    int jump_endif_from_true = emit_jump(_code, OPCODE_JUMP_FORWARD);
                    // false?
                    label(_code, jump_start_r);
                    label(_code, jump_start_l);
                    if (fvalue != NULL) {
                        generator_statement(_generator, _code, _scope, fvalue);
                    }
                    label(_code, jump_endif_from_true);
                } else {
                    generator_expression(_generator, _code, _scope, cond_l); // cond_l
                    int jump_start_l = emit_jump(_code, OPCODE_POP_JUMP_IF_TRUE);
                    // If left is false, then evaluate right
                    generator_expression(_generator, _code, _scope, cond_r); // cond_r
                    int jump_start_r = emit_jump(_code, OPCODE_POP_JUMP_IF_FALSE);
                    // true
                    label(_code, jump_start_l);
                    generator_statement(_generator, _code, _scope, tvalue); // tvalue   
                    // Jump to endif from true
                    int jump_endif_from_true = emit_jump(_code, OPCODE_JUMP_FORWARD);
                    // false?
                    label(_code, jump_start_r);
                    if (fvalue != NULL) {
                        generator_statement(_generator, _code, _scope, fvalue); // fvalue
                    }
                    // Jump to endif from false
                    label(_code, jump_endif_from_true);
                }
            }
            break;
        }
        case AstWhileStatement: {
            ast_node_t* cond = _statement->ast0;
            ast_node_t* body = _statement->ast1;
            if (!cond || !body) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _statement->position, 
                    "while statement must have a condition and a body"
                );
            }
            scope_t* while_scope = scope_new(_scope, ScopeTypeLoop);
            size_t loop_start = here(_code);
            if (generator_is_constant_node(cond) || !generator_is_logical_expression(cond)) {
                // Condition
                generator_expression(_generator, _code, _scope, cond);
                // Jump if false
                int jump_endwhile_if_false = emit_jump(_code, OPCODE_POP_JUMP_IF_FALSE);
                // Body
                generator_statement(_generator, _code, while_scope, body);
                // Jump to the start of the while loop
                emit_jumpto(_code, OPCODE_ABSOLUTE_JUMP, loop_start);
                // Jump to the end of the while loop
                label(_code, jump_endwhile_if_false);
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
                bool is_logical_and = cond->type == AstLogicalAnd;
                if (is_logical_and) {
                    generator_expression(_generator, _code, _scope, cond_l);
                    int jump_start_l = emit_jump(_code, OPCODE_POP_JUMP_IF_FALSE);
                    // If left is true, then evaluate right
                    generator_expression(_generator, _code, _scope, cond_r);
                    int jump_start_r = emit_jump(_code, OPCODE_POP_JUMP_IF_FALSE);
                    // true
                    generator_statement(_generator, _code, while_scope, body);
                    // Jump to the start of the while loop
                    emit_jumpto(_code, OPCODE_ABSOLUTE_JUMP, loop_start);
                    // Jump to the end of the while loop
                    label(_code, jump_start_l);
                    label(_code, jump_start_r);
                } else {
                    generator_expression(_generator, _code, _scope, cond_l);
                    int jump_start_l = emit_jump(_code, OPCODE_POP_JUMP_IF_TRUE);
                    // If left is false, then evaluate right
                    generator_expression(_generator, _code, _scope, cond_r);
                    int jump_start_r = emit_jump(_code, OPCODE_POP_JUMP_IF_FALSE);
                    // true
                    label(_code, jump_start_l);
                    generator_statement(_generator, _code, while_scope, body);
                    // Jump to the start of the while loop
                    emit_jumpto(_code, OPCODE_ABSOLUTE_JUMP, loop_start);
                    // false
                    label(_code, jump_start_r);
                }
            }
            scope_free(while_scope);
            break;
        }
        case AstDoWhileStatement: {
            ast_node_t* cond = _statement->ast0;
            ast_node_t* body = _statement->ast1;
            if (!cond || !body) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _statement->position, 
                    "do while statement must have a condition and a body"
                );
            }
            scope_t* do_while_scope = scope_new(_scope, ScopeTypeLoop);
            size_t loop_start = here(_code);
            if (generator_is_constant_node(cond) || !generator_is_logical_expression(cond)) {
                // Body
                generator_statement(_generator, _code, do_while_scope, body);
                // Condition
                generator_expression(_generator, _code, _scope, cond);
                // Jump if false
                int jump_endwhile_if_false = emit_jump(_code, OPCODE_POP_JUMP_IF_FALSE);
                // Jump to the start of the do while loop
                emit_jumpto(_code, OPCODE_ABSOLUTE_JUMP, loop_start);
                // Jump to the end of the do while loop
                label(_code, jump_endwhile_if_false);
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
                bool is_logical_and = cond->type == AstLogicalAnd;
                if (is_logical_and) {
                    // Body
                    generator_statement(_generator, _code, do_while_scope, body);
                    // Condition
                    generator_expression(_generator, _code, _scope, cond_l);
                    int jump_start_l = emit_jump(_code, OPCODE_POP_JUMP_IF_FALSE);
                    // If left is true, then evaluate right
                    generator_expression(_generator, _code, _scope, cond_r);
                    int jump_start_r = emit_jump(_code, OPCODE_POP_JUMP_IF_FALSE);
                    // Loop
                    emit_jumpto(_code, OPCODE_ABSOLUTE_JUMP, loop_start);
                    // Jump to the end of the do while loop
                    label(_code, jump_start_l);
                    label(_code, jump_start_r);
                } else {
                    // Body
                    generator_statement(_generator, _code, do_while_scope, body);
                    // Condition
                    generator_expression(_generator, _code, _scope, cond_l);
                    int jump_start_l = emit_jump(_code, OPCODE_POP_JUMP_IF_FALSE);
                    // If left is false, then evaluate right
                    generator_expression(_generator, _code, _scope, cond_r);
                    int jump_start_r = emit_jump(_code, OPCODE_POP_JUMP_IF_FALSE);
                    // Loop
                    emit_jumpto(_code, OPCODE_ABSOLUTE_JUMP, loop_start);
                    // Jump to the end of the do while loop
                    label(_code, jump_start_l);
                    label(_code, jump_start_r);
                }
            }
            scope_free(do_while_scope);
            break;
        }
        case AstForStatement: {
            ast_node_t* initializer = _statement->ast0;
            ast_node_t* iterable    = _statement->ast1;
            ast_node_t* body        = _statement->ast2;
            if (initializer == NULL) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _statement->position, 
                    "for statement must have an initializer"
                );
            }
            if (iterable == NULL) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _statement->position, 
                    "for statement must have an iterable"
                );
            }
            if (body == NULL) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _statement->position, 
                    "for statement must have a body"
                );
            }
            scope_t* for_scope = scope_new(_scope, ScopeTypeLoop);
            // Emit the iterable
            generator_expression(_generator, _code, _scope, iterable); // iterable  
            // Emit get iterator
            int jump_if_not_iterable = emit_jump(_code, OPCODE_GET_ITERATOR_OR_JUMP);

            // Check if has next
            int loop_start = here(_code);
            int jump_if_no_next = emit_jump(_code, OPCODE_HAS_NEXT);

            // Emit the initializer
            if (initializer->type == AstName) {
                emit(_code, OPCODE_GET_NEXT_VALUE);

                emit(_code, OPCODE_STORE_NAME);
                emit_string(_code, initializer->str0);

                if (scope_has(for_scope, initializer->str0, false)) {
                    __THROW_ERROR(
                        _generator->fpath, 
                        _generator->fdata, 
                        initializer->position, 
                        "for statement must have a valid initializer"
                    );
                }
                // Save into symbol table
                scope_value_t symbol = {
                    .name      = initializer->str0,
                    .is_const  = true,
                    .is_global = true,
                    .position  = initializer->position
                };
                scope_put(for_scope, initializer->str0, symbol);
            } else if (initializer->type == AstForMultipleInitializer) {
                emit(_code, OPCODE_GET_NEXT_KEY_VALUE);
                ast_node_t* init_l = initializer->ast0;
                ast_node_t* init_r = initializer->ast1;
                if (init_l == NULL) {
                    __THROW_ERROR(
                        _generator->fpath, 
                        _generator->fdata, 
                        initializer->position, 
                        "for statement must have a valid initializer"
                    );
                }
                if (init_r == NULL) {
                    __THROW_ERROR(
                        _generator->fpath, 
                        _generator->fdata, 
                        initializer->position, 
                        "for statement must have a valid initializer"
                    );
                }
                if (init_l->type != AstName || init_r->type != AstName) {
                    __THROW_ERROR(
                        _generator->fpath, 
                        _generator->fdata, 
                        initializer->position, 
                        "for statement must have a valid initializer"
                    );
                }
                // For Key
                emit(_code, OPCODE_STORE_NAME);
                emit_string(_code, init_l->str0);

                // For Value
                emit(_code, OPCODE_STORE_NAME);
                emit_string(_code, init_r->str0); // init_r->str0
                
                // Check if the symbol is already defined
                if (scope_has(for_scope, init_l->str0, false)) {
                    __THROW_ERROR(
                        _generator->fpath, 
                        _generator->fdata, 
                        initializer->position, 
                        "for statement must have a valid initializer"
                    );
                }

                scope_value_t symbol0 = {
                    .name      = init_l->str0,
                    .is_const  = true,
                    .is_global = true,
                    .position  = init_l->position
                };
                scope_put(for_scope, init_l->str0, symbol0);

                if (scope_has(for_scope, init_r->str0, false)) {
                    __THROW_ERROR(
                        _generator->fpath, 
                        _generator->fdata, 
                        initializer->position, 
                        "for statement must have a valid initializer"
                    );
                }
                // Save into symbol table
                scope_value_t symbol1 = {
                    .name      = init_r->str0,
                    .is_const  = true,
                    .is_global = true,
                    .position  = init_r->position
                };
                scope_put(for_scope, init_r->str0, symbol1);
            } else {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    initializer->position, 
                    "for statement must have a valid initializer"
                );
            }
            // Emit the body
            generator_statement(_generator, _code, for_scope, body);
            // Jump backward to the has next address
            emit_jumpto(_code, OPCODE_ABSOLUTE_JUMP, loop_start);
            // Jump here if no next
            label(_code, jump_if_no_next);
            // Emit pop top to pop iterator
            emit(_code, OPCODE_POPTOP);
            // Jump here if not iterable
            label(_code, jump_if_not_iterable);
            // Free the scope
            scope_free(for_scope);
            break;
        }
        case AstReturnStatement: {
            bool is_func = false, is_catch = false;
            if (!(is_func = scope_is_function(_scope)) && !(is_catch = scope_is_catch(_scope))) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _statement->position, 
                    "return statement must be in a function"
                );
            }
            // Find function scope and set returned flag
            scope_t* current;
            if (is_func) {
                for (current = _scope; current->type != ScopeTypeFunction; current = current->parent);
            } else if (is_catch) {
                for (current = _scope; current->type != ScopeTypeCatch; current = current->parent);
            }
            
            current->is_returned = true;
            // Generate return value and return opcode
            ast_node_t* expr = _statement->ast0;
            if (expr != NULL) {
                generator_expression(_generator, _code, _scope, expr);
            } else {
                emit(_code, OPCODE_LOAD_NULL);
            }
            emit(_code, OPCODE_RETURN);
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
            generator_expression(_generator, _code, _scope, _statement->ast0);
            emit(_code, OPCODE_POPTOP);
            break;
        }
        case AstClass: {
            ast_node_t* name = _statement->ast0;
            ast_node_t* super = _statement->ast1;
            ast_node_list_t body = _statement->array0;
            // Validate scope
            if (!scope_is_global(_scope)) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _statement->position, 
                    "class declaration must be in the global scope"
                );
            }
            if (name == NULL) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _statement->position, 
                    "class name must be a valid identifier"
                );
            }
            if (name->type != AstName) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _statement->position, 
                    "class name must be a valid identifier"
                );
            }
            if (body == NULL) {
                __THROW_ERROR(
                    _generator->fpath, 
                    _generator->fdata, 
                    _statement->position, 
                    "class must have a body"
                );
            }
            scope_t* local_scope = scope_new(_scope, ScopeTypeLocal);
            scope_t* class_scope = scope_new(local_scope, ScopeTypeClass);
            // Create class code
            code_t* class_code = code_new_block(
                string_allocate(_generator->fpath),
                string_allocate(name->str0),
                (uint8_t*) malloc(sizeof(uint8_t)),
                0
            );
            
            // Emit the setup class opcode
            emit(_code, OPCODE_SETUP_CLASS);
            // Emit the begin class opcode
            emit(_code, OPCODE_BEGIN_CLASS);
            emit_memory(_code, (void*) class_code);

            int property_count = 0;
            for (size_t i = 0; body[i] != NULL; i++) {
                ast_node_t* statement = body[i];
                
                if (statement->type == AstFunctionNode || statement->type == AstAsyncFunctionNode) {
                    property_count++;
                    ast_node_t* name = statement->ast0;
                    if (name == NULL) {
                        __THROW_ERROR(
                            _generator->fpath, 
                            _generator->fdata, 
                            statement->position, 
                            "function name must be a valid identifier"
                        );
                    }
                    if (name->type != AstName) {
                        __THROW_ERROR(
                            _generator->fpath, 
                            _generator->fdata, 
                            statement->position, 
                            "function name must be a valid identifier"
                        );
                    }
                    generator_function(_generator, class_code, class_scope, statement); // only function can use class scope
                    emit(class_code, OPCODE_DUPTOP);

                    emit(class_code, OPCODE_STORE_NAME);
                    emit_string(class_code, name->str0);

                    // Emit function name as key
                    emit(class_code, OPCODE_LOAD_STRING);
                    emit_string(class_code, name->str0);

                    if (scope_has(local_scope, name->str0, false)) {
                        __THROW_ERROR(
                            _generator->fpath, 
                            _generator->fdata, 
                            statement->position, 
                            "function %s is already defined", name->str0
                        );
                    }

                    // Save into symbol table
                    scope_value_t symbol = {
                        .name      = name->str0,
                        .is_const  = false,
                        .is_global = true,
                        .position  = name->position
                    };
                    scope_put(local_scope, name->str0, symbol);
                } else if (statement->type == AstVarStatement) {
                    ast_node_list_t names = statement->array0;
                    ast_node_list_t values = statement->array1;
                    for (size_t i = 0; names[i] != NULL; i++) {
                        ast_node_t* name = names[i];
                        ast_node_t* value = values[i];
                        if (name == NULL) {
                            __THROW_ERROR(
                                _generator->fpath, 
                                _generator->fdata, 
                                statement->position, 
                                "variable name must be a valid identifier"
                            );
                        }
                        if (name->type != AstName) {
                            __THROW_ERROR(
                                _generator->fpath, 
                                _generator->fdata, 
                                statement->position, 
                                "variable name must be a valid identifier"
                            );
                        }
                        if (scope_has(local_scope, name->str0, false)) {
                            __THROW_ERROR(
                                _generator->fpath, 
                                _generator->fdata, 
                                statement->position, 
                                "variable %s is already defined", name->str0
                            );
                        }
                        ++property_count;
                        if (value == NULL) {
                            emit(class_code, OPCODE_LOAD_NULL);
                        } else {
                            generator_expression(
                                _generator, 
                                class_code, 
                                ((value->type == AstFunctionExpression)
                                ? class_scope
                                : local_scope),
                                value
                            );
                        }
                        emit(class_code, OPCODE_DUPTOP);

                        emit(class_code, OPCODE_STORE_NAME);
                        emit_string(class_code, name->str0);

                        // Emit the store name opcode
                        emit(class_code, OPCODE_LOAD_STRING);
                        emit_string(class_code, name->str0);

                        if (scope_has(local_scope, name->str0, false)) {
                            __THROW_ERROR(
                                _generator->fpath, 
                                _generator->fdata, 
                                statement->position, 
                                "variable %s is already defined", name->str0
                            );
                        }

                        // Save into symbol table
                        scope_value_t symbol = {
                            .name      = name->str0,
                            .is_const  = statement->type == AstConstStatement,
                            .is_global = true,
                            .position  = name->position
                        };
                        scope_put(local_scope, name->str0, symbol);
                    }
                } else if (statement->type == AstLocalStatement || statement->type == AstConstStatement) {
                    __THROW_ERROR(
                        _generator->fpath, 
                        _generator->fdata, 
                        statement->position, 
                        "declaration must be a function or a variable declaration"
                    );
                } else {
                    generator_statement(_generator, class_code, local_scope, statement);
                }
            }
            emit(class_code, OPCODE_LOAD_OBJECT);
            emit_int(class_code, property_count);

            // Emit Store Class
            emit(_code, OPCODE_STORE_CLASS);
            emit_string(_code, name->str0);
            if (super != NULL) {
                generator_expression(_generator, _code, _scope, super);
                emit(_code, OPCODE_EXTEND_CLASS);
            }
            // Emit the pop top opcode
            emit(_code, OPCODE_POPTOP);
            // Free the class scope
            scope_free(local_scope);
            scope_free(class_scope);
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
            size_t param_count;
            for (param_count = 0; params[param_count] != NULL; param_count++);

            code_t* _func = code_new_function(
                string_allocate(_generator->fpath),
                string_allocate(name->str0),
                false,
                is_async,
                param_count,
                (uint8_t*) malloc(sizeof(uint8_t)),
                0
            );
            // Make function
            emit(_code, OPCODE_SETUP_FUNCTION);
            emit(_code, OPCODE_BEGIN_FUNCTION);
            emit_memory(_code, (void*) _func);
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
                if (scope_has(local_scope, param->str0, false)) {
                    __THROW_ERROR(
                        _generator->fpath, 
                        _generator->fdata, 
                        param->position, 
                        "function parameter %s is already defined", param->str0
                    );
                }
                // Save into symbol table
                scope_value_t symbol = {
                    .name      = param->str0,
                    .is_const  = false,
                    .is_global = true,
                    .position  = param->position
                };
                scope_put(local_scope, param->str0, symbol);
                // Emit the store name opcode
                emit(_func, OPCODE_STORE_NAME);
                emit_string(_func, param->str0);
            }
            // Compile body
            bool has_visible_return = false;
            for (size_t i = 0; body[i] != NULL; i++) {
                ast_node_t* statement = body[i];
                if (body[i]->type == AstReturnStatement && has_visible_return) {
                    break;
                }
                if (body[i]->type == AstReturnStatement) has_visible_return = true;
                generator_statement(_generator, _func, local_scope, statement);
            }
            if (!has_visible_return) {
                // Emit the return opcode
                emit(_func, OPCODE_LOAD_NULL);
                emit(_func, OPCODE_RETURN);
            }
            // Save into symbol table
            scope_value_t symbol = {
                .name      = name->str0,
                .is_const  = false,
                .is_global = true,
                .position  = name->position
            };
            scope_put(_scope, name->str0, symbol);
            // Save captures
            if (function_scope->capture_count > 0) {
                // Emit opcode save captures
                emit(_code, OPCODE_SAVE_CAPTURES);
                emit_int(_code, function_scope->capture_count);
                for (size_t i = 0; i < function_scope->capture_count; i++) {
                    emit_string(_code, function_scope->captures[i]);
                }
            }
            // Emit the store name opcode
            emit(_code, OPCODE_STORE_NAME);
            emit_string(_code, name->str0);
            // Free the function scope
            scope_free(local_scope);
            scope_free(function_scope);
            break;
        }
        case AstBlockStatement: {
            ast_node_list_t statements = _statement->array0;
            code_t* _block = code_new_block(
                string_allocate(_generator->fpath),
                string_allocate("block"),
                (uint8_t*) malloc(sizeof(uint8_t)),
                0
            );
            scope_t* block_scope = scope_new(_scope, ScopeTypeLocal);
            // Setup block and reserve space for metadata
            emit(_code, OPCODE_SETUP_BLOCK);
            emit(_code, OPCODE_BEGIN_BLOCK);
            emit_memory(_code, (void*) _block);
            // Compile all statements in block
            for (size_t i = 0; statements[i] != NULL; i++) {
                generator_statement(_generator, _block, block_scope, statements[i]);
            }
            // Emit complete and finalize
            emit(_block, OPCODE_LOAD_NULL);
            emit(_block, OPCODE_COMPLETE_BLOCK);
            // Free the block scope
            scope_free(block_scope);
            break;
        }
        default:
            PD("unsupported statement type %d.", _statement->type);
    }
}

INTERNAL code_t* generator_program(generator_t* _generator, ast_node_t* _program) {
    ast_node_list_t children = _program->array0;
    code_t* _block = code_new_module(
        string_allocate(_generator->fpath),
        path_get_file_name(_generator->fpath)
    );
    scope_t* scope = scope_new(NULL, ScopeTypeGlobal);
    for (size_t i = 0; children[i] != NULL; i++) {
        ast_node_t* child = children[i];
        generator_statement(_generator, _block, scope, child);
    }
    // write the bytecode to the file
    emit(_block, OPCODE_LOAD_NULL);
    emit(_block, OPCODE_RETURN);
    // Free the scope
    scope_free(scope);
    ast_node_free_all(_program);
    return _block;
}

// -----------------------------

DLLEXPORT generator_t* generator_new(char* _fpath, char* _fdata) {
    generator_t* generator = (generator_t*) malloc(sizeof(generator_t));
    ASSERTNULL(generator, "failed to allocate memory for generator");
    generator->fpath = string_allocate(_fpath);
    generator->fdata = string_allocate(_fdata);
    generator->fsize = strlen(_fdata);
    generator->bsize = 0;
    generator->bytecode = (uint8_t*) malloc(sizeof(uint8_t) * 1);
    ASSERTNULL(generator->bytecode, "failed to allocate memory for bytecode");
    // Return instance
    return generator;
}

DLLEXPORT code_t* generator_generate(generator_t* _generator, ast_node_t* _program) {
    return generator_program(_generator, _program);
}

DLLEXPORT void generator_free(generator_t* _generator) {
    free(_generator->fpath);
    free(_generator->fdata);
    free(_generator);
}

#endif