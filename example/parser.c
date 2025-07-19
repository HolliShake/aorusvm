#include "parser.h"

parser_t* parser_new(char* _path, char* _data) {
    parser_t* parser = (parser_t*) malloc(sizeof(parser_t));
    if (parser == NULL) {
        PD("Failed to allocate memory for parser");
    }
    parser->fpath = _path;
    parser->fdata = _data;
    parser->tokenizer = tokenizer_new(_path, _data);
    parser->current = NULL;
    return parser;
}

#define CHECKV(_value) parser_check(_parser, false, _parser->current->type, _value)
#define CHECKT(_ttype)  parser_check(_parser, true, _ttype, NULL)
#define ACCEPTV(_value) parser_accept(_parser, false, _parser->current->type, _value)
#define ACCEPTT(_ttype) parser_accept(_parser, true, _ttype, NULL)

bool parser_check(parser_t* _parser, bool _is_type, token_type_t _type, char* _value) {
    if (_is_type) {
        return _parser->current->type == _type;
    } else {
        return (
            (strcmp(_parser->current->value, _value) == 0) && 
            (CHECKT(TTKEY) || CHECKT(TTIDN) || CHECKT(TTSYM))
        );
    }
}

void parser_accept(parser_t* _parser, bool _is_type, token_type_t _type, char* _value) {
    if (parser_check(_parser, _is_type, _type, _value)) {
        _parser->current = tokenizer_next(_parser->tokenizer);
        return;
    }
    __THROW_ERROR(
        _parser->fpath,
        _parser->fdata,
        _parser->current->position,
        "unexpected token: '%s'", _parser->current->value
    );
}

ast_node_t* parser_expression(parser_t* _parser);
ast_node_t* parser_mandatory_expression(parser_t* _parser);
ast_node_t* parser_statement(parser_t* _parser);

ast_node_t* parser_terminal(parser_t* _parser) {
    if (CHECKT(TTIDN)) {
        ast_node_t* node = ast_name_node(
            _parser->current->position,
            _parser->current->value
        );
        ACCEPTT(TTIDN);
        return node;
    } else if (CHECKT(TTINT)) {
        ast_node_t* node = ast_int_node(
            _parser->current->position,
            atoi(_parser->current->value)
        );
        ACCEPTT(TTINT);
        return node;
    } else if (CHECKT(TTNUM)) {
        ast_node_t* node = ast_double_node(
            _parser->current->position,
            strtod(_parser->current->value, NULL)
        );
        ACCEPTT(TTNUM);
        return node;
    } else if (CHECKT(TTSTR)) {
        ast_node_t* node = ast_string_node(
            _parser->current->position,
            _parser->current->value
        );
        ACCEPTT(TTSTR);
        return node;
    } else if (CHECKT(TTKEY) && CHECKV(KEY_TRUE)) {
        ast_node_t* node = ast_boolean_node(
            _parser->current->position,
            true
        );
        ACCEPTT(TTKEY);
        return node;
    } else if (CHECKT(TTKEY) && CHECKV(KEY_FALSE)) {
        ast_node_t* node = ast_boolean_node(
            _parser->current->position,
            false
        );
        ACCEPTT(TTKEY);
        return node;
    } else if (CHECKT(TTKEY) && CHECKV(KEY_NULL)) {
        ast_node_t* node = ast_null_node(
            _parser->current->position
        );
        ACCEPTT(TTKEY);
        return node;
    } else if (CHECKT(TTKEY) && CHECKV(KEY_THIS)) {
        ast_node_t* node = ast_this_node(_parser->current->position);
        ACCEPTT(TTKEY);
        return node;
    } else if (CHECKT(TTKEY) && CHECKV(KEY_SUPER)) {
        ast_node_t* node = ast_super_node(_parser->current->position);
        ACCEPTT(TTKEY);
        return node;
    } else {
        return NULL;
    }
}

ast_node_t* parser_array(parser_t* _parser) {
    position_t* start = _parser->current->position, *ended = start;
    ACCEPTV(LBRACE);
    size_t index = 0;
    ast_node_list_t elements = (ast_node_list_t) malloc(sizeof(ast_node_t*));
    elements[0] = NULL;

    ast_node_t* element = parser_expression(_parser);

    if (element != NULL) {
        elements[index++] = element;
        elements = (ast_node_list_t) realloc(elements, (sizeof(ast_node_t*) * (index + 1)));
        elements[index] = NULL;

        while (CHECKV(COMMA)) {
            ACCEPTV(COMMA);
            ast_node_t* element = parser_expression(_parser);
            if (element == NULL) {
                __THROW_ERROR(
                    _parser->fpath,
                    _parser->fdata,
                    _parser->current->position,
                    "array element expected"
                );
            }
            elements[index++] = element;
            elements = (ast_node_list_t) realloc(elements, (sizeof(ast_node_t*) * (index + 1)));
            elements[index] = NULL;
        }
    }
    ended = _parser->current->position;
    ACCEPTV(RBRACE);
    return ast_array_node(position_merge(start, ended), elements);
}

ast_node_t* parser_object_property(parser_t* _parser) {
    ast_node_t* key = parser_expression(_parser);
    if (key == NULL) {
        return NULL;
    }
    if (!CHECKV(COLON)) {
        return key;
    }
    ACCEPTV(COLON);
    ast_node_t* value = parser_expression(_parser);
    if (value == NULL) {
        __THROW_ERROR(
            _parser->fpath,
            _parser->fdata,
            _parser->current->position,
            "object property value expected"
        );
    }
    return ast_object_property_node(position_merge(ast_position(key), ast_position(value)), key, value);
}

ast_node_t* parser_object(parser_t* _parser) {
    position_t* start = _parser->current->position, *ended = start;
    size_t index = 0;
    ast_node_list_t properties = (ast_node_list_t) malloc(sizeof(ast_node_t*));
    properties[0] = NULL;
    ACCEPTV(LBRACKET);
    ast_node_t* property = parser_object_property(_parser);
    if (property != NULL) {
        properties[index++] = property;
        properties = (ast_node_list_t) realloc(properties, (sizeof(ast_node_t*) * (index + 1)));
        properties[index] = NULL;

        while (CHECKV(COMMA)) {
            ACCEPTV(COMMA);
            property = parser_object_property(_parser);
            if (property == NULL) {
                __THROW_ERROR(
                    _parser->fpath,
                    _parser->fdata,
                    _parser->current->position,
                    "object property expected"
                );
            }
            properties[index++] = property;
            properties = (ast_node_list_t) realloc(properties, (sizeof(ast_node_t*) * (index + 1)));
            properties[index] = NULL;
        }
    }
    ended = _parser->current->position;
    ACCEPTV(RBRACKET);
    return ast_object_node(position_merge(start, ended), properties);
}

ast_node_t* parser_function_expression(parser_t* _parser) {
    position_t* start = _parser->current->position, *ended = start;
    ACCEPTV(KEY_FUNC);
    ACCEPTV(LPAREN);
    size_t index = 0;
    ast_node_list_t parameters = (ast_node_list_t) malloc(sizeof(ast_node_t*));
    parameters[0] = NULL;

    ast_node_t* parameter = parser_terminal(_parser);

    if (parameter != NULL) {
        parameters[index++] = parameter;
        parameters = (ast_node_list_t) realloc(parameters, (sizeof(ast_node_t*) * (index + 1)));
        parameters[index] = NULL;
        while (CHECKV(COMMA)) {
            ACCEPTV(COMMA);
            ast_node_t* parameter = parser_terminal(_parser);
            if (parameter == NULL) {
                __THROW_ERROR(
                    _parser->fpath,
                    _parser->fdata,
                    _parser->current->position,
                    "function parameter expected"
                );
            }
            parameters[index++] = parameter;
            parameters = (ast_node_list_t) realloc(parameters, (sizeof(ast_node_t*) * (index + 1)));
            parameters[index] = NULL;
        }
    }
    ACCEPTV(RPAREN);
    index = 0;
    ast_node_list_t body = (ast_node_list_t) malloc(sizeof(ast_node_t*));
    body[0] = NULL;

    ACCEPTV(LBRACKET);

    ast_node_t* statement = parser_statement(_parser);

    while (statement != NULL) {
        body[index++] = statement;
        body = (ast_node_list_t) realloc(body, (sizeof(ast_node_t*) * (index + 1)));
        body[index] = NULL;
        statement = parser_statement(_parser);
    }
    ended = _parser->current->position;
    ACCEPTV(RBRACKET);
    return ast_function_expression_node(position_merge(start, ended), parameters, body);
}

ast_node_t* parser_group(parser_t* _parser) {
    if (CHECKV(LBRACE)) {
        return parser_array(_parser);
    } else if (CHECKV(LBRACKET)) {
        return parser_object(_parser);
    } else if (CHECKV(KEY_FUNC)) {
        return parser_function_expression(_parser);
    } else if (CHECKV(LPAREN)) {
        ACCEPTV(LPAREN);
        ast_node_t* node = parser_mandatory_expression(_parser);
        ACCEPTV(RPAREN);
        return node;
    } else {
        return parser_terminal(_parser);
    }
}

ast_node_t* parser_member_or_call(parser_t* _parser) {
    position_t* start = _parser->current->position, *ended = start;
    ast_node_t* node = parser_group(_parser);
    if (node == NULL) {
        return NULL;
    }
    while (CHECKV(DOT) || CHECKV(LBRACE) || CHECKV(LPAREN)) {
        if (CHECKV(DOT)) {
            ACCEPTV(DOT);
            ast_node_t* member = parser_terminal(_parser);
            if (member == NULL) {
                __THROW_ERROR(
                    _parser->fpath,
                    _parser->fdata,
                    _parser->current->position,
                    "member access expected"
                );
            }
            ended = ast_position(member);
            node = ast_member_access_node(position_merge(start, ended), node, member);
        } else if (CHECKV(LBRACE)) {
            ACCEPTV(LBRACE);
            ast_node_t* index = parser_expression(_parser);
            if (index == NULL) {
                __THROW_ERROR(
                    _parser->fpath,
                    _parser->fdata,
                    _parser->current->position,
                    "index expression expected"
                );
            }
            ended = _parser->current->position;
            ACCEPTV(RBRACE);
            node = ast_index_node(position_merge(start, ended), node, index);
        } else if (CHECKV(LPAREN)) {
            size_t index = 0;
            ast_node_list_t arguments = (ast_node_list_t) malloc(sizeof(ast_node_t*));
            arguments[0] = NULL;
            ACCEPTV(LPAREN);
            ast_node_t* argument = parser_expression(_parser);
            if (argument != NULL) {
                arguments[index++] = argument;
                arguments = (ast_node_list_t) realloc(arguments, (sizeof(ast_node_t*) * (index + 1)));
                arguments[index] = NULL;
                while (CHECKV(COMMA)) {
                    ACCEPTV(COMMA);
                    ast_node_t* argument = parser_expression(_parser);
                    if (argument == NULL) {
                        __THROW_ERROR(
                            _parser->fpath,
                            _parser->fdata,
                            _parser->current->position,
                            "function argument expected"
                        );
                    }
                    arguments[index++] = argument;
                    arguments = (ast_node_list_t) realloc(arguments, (sizeof(ast_node_t*) * (index + 1)));
                    arguments[index] = NULL;
                }
            }
            ended = _parser->current->position;
            ACCEPTV(RPAREN);
            node = ast_call_node(position_merge(start, ended), node, arguments);
        }
    }
    return node;
}

ast_node_t* parser_unary(parser_t* _parser) {
    position_t* start = _parser->current->position, *ended = start;
    if (CHECKV(PLUSPLUS)) {
        ACCEPTV(PLUSPLUS);
        ast_node_t* node = parser_unary(_parser);
        if (node == NULL) {
            __THROW_ERROR(
                _parser->fpath,
                _parser->fdata,
                _parser->current->position,
                "missing operand for ++"
            );
        }
        ended = ast_position(node);
        return ast_unary_plus_node(position_merge(start, ended), node);
    } else if (CHECKV(SPREAD)) {
        ACCEPTV(SPREAD);
        ast_node_t* node = parser_unary(_parser);
        if (node == NULL) {
            __THROW_ERROR(
                _parser->fpath,
                _parser->fdata,
                _parser->current->position,
                "missing operand for ..."
            );
        }
        ended = ast_position(node);
        return ast_unary_spread_node(position_merge(start, ended), node);
    } else if (CHECKV(KEY_NEW)) {
        ACCEPTV(KEY_NEW);
        ast_node_t* node = parser_member_or_call(_parser);
        if (node == NULL) {
            __THROW_ERROR(
                _parser->fpath,
                _parser->fdata,
                _parser->current->position,
                "missing operand for new"
            );
        }
        ended = ast_position(node);
        return ast_new_node(position_merge(start, ended), node);
    }
    return parser_member_or_call(_parser);
}

ast_node_t* parser_multiplicative(parser_t* _parser) {
    ast_node_t* node = parser_unary(_parser);
    if (node == NULL) {
        return NULL;
    }
    while (CHECKV(MULTIPLY) || CHECKV(DIVIDE) || CHECKV(MODULO)) {
        char* op = _parser->current->value;
        ACCEPTV(op);
        ast_node_t* right = parser_unary(_parser);
        if (right == NULL) {
            __THROW_ERROR(
                _parser->fpath,
                _parser->fdata,
                _parser->current->position,
                "missing right operand for %s", op
            );
        }
        if (strcmp(op, MULTIPLY) == 0) {
            node = ast_binary_mul_node(
                position_merge(ast_position(node), ast_position(right)), 
                node, 
                right
            );
        } else if (strcmp(op, DIVIDE) == 0) {
            node = ast_binary_div_node(
                position_merge(ast_position(node), ast_position(right)), 
                node, 
                right
            );
        } else if (strcmp(op, MODULO) == 0) {
            node = ast_binary_mod_node(
                position_merge(ast_position(node), ast_position(right)), 
                node, 
                right
            );
        }
    }
    return node;
}

ast_node_t* parser_additive(parser_t* _parser) {
    ast_node_t* node = parser_multiplicative(_parser);
    if (node == NULL) {
        return NULL;
    }
    while (CHECKV(PLUS) || CHECKV(MINUS)) {
        char* op = _parser->current->value;
        ACCEPTV(op);
        ast_node_t* right = parser_multiplicative(_parser);
        if (right == NULL) {
            __THROW_ERROR(
                _parser->fpath,
                _parser->fdata,
                _parser->current->position,
                "missing right operand for %s", op
            );
        }
        if (strcmp(op, PLUS) == 0) {
            node = ast_binary_add_node(
                position_merge(ast_position(node), ast_position(right)), 
                node, 
                right
            );
        } else if (strcmp(op, MINUS) == 0) {
            node = ast_binary_sub_node(
                position_merge(ast_position(node), ast_position(right)), 
                node, 
                right
            );
        }
    }
    return node;
}

ast_node_t* parser_shifting(parser_t* _parser) {
    ast_node_t* node = parser_additive(_parser);
    if (node == NULL) {
        return NULL;
    }
    while (CHECKV(SHIFT_LEFT) || CHECKV(SHIFT_RIGHT)) {
        char* op = _parser->current->value;
        ACCEPTV(op);
        ast_node_t* right = parser_additive(_parser);
        if (right == NULL) {
            __THROW_ERROR(
                _parser->fpath,
                _parser->fdata,
                _parser->current->position,
                "missing right operand for %s", op
            );
        }
        if (strcmp(op, SHIFT_LEFT) == 0) {
            node = ast_binary_shl_node(
                position_merge(ast_position(node), ast_position(right)), 
                node, 
                right
            );
        } else if (strcmp(op, SHIFT_RIGHT) == 0) {
            node = ast_binary_shr_node(
                position_merge(ast_position(node), ast_position(right)), 
                node, 
                right
            );
        }
    }
    return node;
}

ast_node_t* parser_relational(parser_t* _parser) {
    ast_node_t* node = parser_shifting(_parser);
    if (node == NULL) {
        return NULL;
    }
    while (CHECKV(LESS) || CHECKV(LESS_EQUAL) || CHECKV(GREATER) || CHECKV(GREATER_EQUAL)) {
        char* op = _parser->current->value;
        ACCEPTV(op);
        ast_node_t* right = parser_shifting(_parser);
        if (right == NULL) {
            __THROW_ERROR(
                _parser->fpath,
                _parser->fdata,
                _parser->current->position,
                "missing right operand for %s", op
            );
        }
        if (strcmp(op, LESS) == 0) {
            node = ast_cmp_lt_node(
                position_merge(ast_position(node), ast_position(right)), 
                node, 
                right
            );
        } else if (strcmp(op, LESS_EQUAL) == 0) {
            node = ast_cmp_lte_node(
                position_merge(ast_position(node), ast_position(right)), 
                node, 
                right
            );
        } else if (strcmp(op, GREATER) == 0) {
            node = ast_cmp_gt_node(
                position_merge(ast_position(node), ast_position(right)), 
                node, 
                right
            );
        } else if (strcmp(op, GREATER_EQUAL) == 0) {
            node = ast_cmp_gte_node(
                position_merge(ast_position(node), ast_position(right)), 
                node, 
                right
            );
        }
    }
    return node;
}

ast_node_t* parser_equality(parser_t* _parser) {
    ast_node_t* node = parser_relational(_parser);
    if (node == NULL) {
        return NULL;
    }
    while (CHECKV(EQUALTO) || CHECKV(NOT_EQUALTO)) {
        char* op = _parser->current->value;
        ACCEPTV(op);
        ast_node_t* right = parser_relational(_parser);
        if (right == NULL) {
            __THROW_ERROR(
                _parser->fpath,
                _parser->fdata,
                _parser->current->position,
                "missing right operand for %s", op
            );
        }
        if (strcmp(op, EQUALTO) == 0) {
            node = ast_cmp_eq_node(
                position_merge(ast_position(node), ast_position(right)), 
                node, 
                right
            );
        } else if (strcmp(op, NOT_EQUALTO) == 0) {
            node = ast_cmp_ne_node(
                position_merge(ast_position(node), ast_position(right)), 
                node, 
                right
            );
        }
    }
    return node;
}

ast_node_t* parser_bitwise(parser_t* _parser) {
    ast_node_t* node = parser_equality(_parser);
    if (node == NULL) {
        return NULL;
    }
    while (CHECKV(BITWISE_AND) || CHECKV(BITWISE_OR) || CHECKV(BITWISE_XOR)) {
        char* op = _parser->current->value;
        ACCEPTV(op);
        ast_node_t* right = parser_equality(_parser);
        if (right == NULL) {
            __THROW_ERROR(
                _parser->fpath,
                _parser->fdata,
                _parser->current->position,
                "missing right operand for %s", op
            );
        }
        if (strcmp(op, BITWISE_AND) == 0) {
            node = ast_binary_and_node(
                position_merge(ast_position(node), ast_position(right)), 
                node, 
                right
            );
        } else if (strcmp(op, BITWISE_OR) == 0) {
            node = ast_binary_or_node(
                position_merge(ast_position(node), ast_position(right)), 
                node, 
                right
            );
        } else if (strcmp(op, BITWISE_XOR) == 0) {
            node = ast_binary_xor_node(
                position_merge(ast_position(node), ast_position(right)), 
                node, 
                right
            );
        }
    }
    return node;
}

ast_node_t* parser_logical(parser_t* _parser) {
    ast_node_t* node = parser_bitwise(_parser);
    if (node == NULL) {
        return NULL;
    }
    while (CHECKV(AND) || CHECKV(OR)) {
        char* op = _parser->current->value;
        ACCEPTV(op);
        ast_node_t* right = parser_bitwise(_parser);
        if (right == NULL) {
            __THROW_ERROR(
                _parser->fpath,
                _parser->fdata,
                _parser->current->position,
                "missing right operand for %s", op
            );
        }
        if (strcmp(op, "&&") == 0) {
            node = ast_logical_and_node(
                position_merge(ast_position(node), ast_position(right)), 
                node, 
                right
            );
        } else if (strcmp(op, "||") == 0) {
            node = ast_logical_or_node(
                position_merge(ast_position(node), ast_position(right)), 
                node, 
                right
            );
        }
    }
    return node;
}

ast_node_t* parser_generator(parser_t* _parser) {
    ast_node_t* node = parser_logical(_parser);
    if (node == NULL) {
        return NULL;
    }
    while (CHECKV(DOTDOT)) {
        char* op = _parser->current->value;
        ACCEPTV(op);
        ast_node_t* right = parser_logical(_parser);
        if (right == NULL) {
            __THROW_ERROR(
                _parser->fpath,
                _parser->fdata,
                _parser->current->position,
                "missing right operand for %s", op
            );
        }
        node = ast_range_node(
            position_merge(ast_position(node), ast_position(right)), 
            node, 
            right
        );
    }
    return node;
}

ast_node_t* parser_catch(parser_t* _parser) {
    position_t* start = _parser->current->position, *ended = start;
    ast_node_t* error = parser_generator(_parser);
    if (error == NULL) {
        return NULL;
    }
    if (!CHECKV(KEY_CATCH)) {
        return error;
    }
    ACCEPTV(KEY_CATCH);
    ACCEPTV(LPAREN);
    ast_node_t* placeholder = parser_terminal(_parser);
    if (placeholder == NULL) {
        __THROW_ERROR(
            _parser->fpath,
            _parser->fdata,
            _parser->current->position,
            "catch placeholder expected"
        );
    }
    ACCEPTV(RPAREN);
    ACCEPTV(LBRACKET);
    size_t index = 0;
    ast_node_list_t body = (ast_node_list_t) malloc(sizeof(ast_node_t*));
    body[0] = NULL;
    ast_node_t* statement = parser_statement(_parser);
    while (statement != NULL) {
        body[index++] = statement;
        body = (ast_node_list_t) realloc(body, (sizeof(ast_node_t*) * (index + 1)));
        body[index] = NULL;
        statement = parser_statement(_parser);
    }
    ended = _parser->current->position;
    ACCEPTV(RBRACKET);
    return ast_catch_node(position_merge(start, ended), error, placeholder, body);
}

ast_node_t* parser_expression(parser_t* _parser) {
    return parser_catch(_parser);
}

ast_node_t* parser_mandatory_expression(parser_t* _parser) {
    ast_node_t* node = parser_expression(_parser);
    if (node == NULL) {
        __THROW_ERROR(
            _parser->fpath,
            _parser->fdata,
            _parser->current->position,
            "expected expression"
        );
    }
    return node;
}

ast_node_t* parser_class_declaration(parser_t* _parser);
ast_node_t* parser_function_declaration(parser_t* _parser);
ast_node_t* parser_variable_declaration(parser_t* _parser, bool _is_const, bool _is_local);
ast_node_t* parser_if_statement(parser_t* _parser);
ast_node_t* parser_while_statement(parser_t* _parser);
ast_node_t* parser_do_while_statement(parser_t* _parser);
ast_node_t* parser_for_statement(parser_t* _parser);
ast_node_t* parser_block_statement(parser_t* _parser);
ast_node_t* parser_return_statement(parser_t* _parser);

ast_node_t* parser_statement(parser_t* _parser) {
    position_t* start = _parser->current->position, *ended = start;
    if (CHECKV(KEY_CLASS)) {
        return parser_class_declaration(_parser);
    } else if (CHECKV(KEY_FUNC)) {
        return parser_function_declaration(_parser);
    } else if (CHECKV(KEY_VAR)) {
        return parser_variable_declaration(_parser, false, false);
    } else if (CHECKV(KEY_CONST)) {
        return parser_variable_declaration(_parser, true, false);
    } else if (CHECKV(KEY_LOCAL)) {
        return parser_variable_declaration(_parser, false, true);
    } else if (CHECKV(KEY_IF)) {
        return parser_if_statement(_parser);
    } else if (CHECKV(KEY_WHILE)) {
        return parser_while_statement(_parser);
    } else if (CHECKV(KEY_DO)) {
        return parser_do_while_statement(_parser);
    } else if (CHECKV(KEY_FOR)) {
        return parser_for_statement(_parser);
    } else if (CHECKV(LBRACKET)) {
        return parser_block_statement(_parser);
    } else if (CHECKV(KEY_RETURN)) {
        return parser_return_statement(_parser);
    }
    /************************************/
    ast_node_t* node = parser_expression(_parser);
    if (node == NULL) {
        return NULL;
    }
    ended = _parser->current->position;
    ACCEPTV(SEMICOLON);
    ast_node_t* expr = ast_expression_statement_node(
        position_merge(start, ended),
        node
    );
    return expr;
}

ast_node_t* parser_class_declaration(parser_t* _parser) {
    position_t* start = _parser->current->position, *ended = start;
    ACCEPTV(KEY_CLASS);
    ast_node_t* name = parser_terminal(_parser);
    if (name == NULL) {
        __THROW_ERROR(
            _parser->fpath,
            _parser->fdata,
            _parser->current->position,
            "class name expected"
        );
    }
    ast_node_t* super = NULL;
    if (CHECKV(KEY_EXTENDS)) {
        ACCEPTV(KEY_EXTENDS);
        super = parser_terminal(_parser);
        if (super == NULL) {
            __THROW_ERROR(
                _parser->fpath,
                _parser->fdata,
                _parser->current->position,
                "super class expected"
            );
        }
    }
    ACCEPTV(LBRACKET);
    size_t index = 0;
    ast_node_list_t body = (ast_node_list_t) malloc(sizeof(ast_node_t*));
    body[0] = NULL;
    ast_node_t* statement = parser_statement(_parser);
    while (statement != NULL) {
        body[index++] = statement;
        body = (ast_node_list_t) realloc(body, (sizeof(ast_node_t*) * (index + 1)));
        body[index] = NULL;
        statement = parser_statement(_parser);
    }
    ended = _parser->current->position;
    ACCEPTV(RBRACKET);
    return ast_class_node(position_merge(start, ended), name, super, body);
}

ast_node_t* parser_function_declaration(parser_t* _parser) {
    position_t* start = _parser->current->position, *ended = start;
    ACCEPTV(KEY_FUNC);
    ast_node_t* name = parser_terminal(_parser);
    if (name == NULL) {
        __THROW_ERROR(
            _parser->fpath,
            _parser->fdata,
            _parser->current->position,
            "function name expected"
        );
    }
    ACCEPTV(LPAREN);
    size_t index = 0;
    ast_node_list_t parameters = (ast_node_list_t) malloc(sizeof(ast_node_t*));
    parameters[0] = NULL;

    ast_node_t* parameter = parser_terminal(_parser);

    if (parameter != NULL) {
        parameters[index++] = parameter;
        parameters = (ast_node_list_t) realloc(parameters, (sizeof(ast_node_t*) * (index + 1)));
        parameters[index] = NULL;
        while (CHECKV(COMMA)) {
            ACCEPTV(COMMA);
            ast_node_t* parameter = parser_terminal(_parser);
            if (parameter == NULL) {
                __THROW_ERROR(
                    _parser->fpath,
                    _parser->fdata,
                    _parser->current->position,
                    "function parameter expected"
                );
            }
            parameters[index++] = parameter;
            parameters = (ast_node_list_t) realloc(parameters, (sizeof(ast_node_t*) * (index + 1)));
            parameters[index] = NULL;
        }
    }
    ACCEPTV(RPAREN);
    index = 0;
    ast_node_list_t body = (ast_node_list_t) malloc(sizeof(ast_node_t*));
    body[0] = NULL;

    ACCEPTV(LBRACKET);

    ast_node_t* statement = parser_statement(_parser);

    while (statement != NULL) {
        body[index++] = statement;
        body = (ast_node_list_t) realloc(body, (sizeof(ast_node_t*) * (index + 1)));
        body[index] = NULL;
        statement = parser_statement(_parser);
    }
    ended = _parser->current->position;
    ACCEPTV(RBRACKET);
    return ast_function_node(position_merge(start, ended), name, parameters, body);
}

ast_node_t* parser_variable_declaration(parser_t* _parser, bool _is_const, bool _is_local) {
    position_t* start = _parser->current->position, *ended = start;
    size_t index0 = 0, index1 = 0;
    ast_node_list_t names = (ast_node_list_t) malloc(sizeof(ast_node_t*));
    ast_node_list_t valus = (ast_node_list_t) malloc(sizeof(ast_node_t*));
    names[0] = NULL;
    valus[0] = NULL;

    if (!_is_const && !_is_local) {
        ACCEPTV(KEY_VAR);
    } else if (_is_const) {
        ACCEPTV(KEY_CONST);
    } else if (_is_local) {
        ACCEPTV(KEY_LOCAL);
    } else {
        __THROW_ERROR(
            _parser->fpath,
            _parser->fdata,
            _parser->current->position,
            "invalid variable declaration"
        );
    }
    ast_node_t* nameN = parser_terminal(_parser);
    ast_node_t* valueN = NULL;

    if (nameN == NULL) {
        __THROW_ERROR(
            _parser->fpath,
            _parser->fdata,
            _parser->current->position,
            "variable name expected"
        );
    }

    names[index0++] = nameN;
    names = (ast_node_list_t) realloc(names, (sizeof(ast_node_t*) * (index0 + 2)));
    names[index0] = NULL;

    if (CHECKV(EQUAL)) {
        ACCEPTV(EQUAL);
        valueN = parser_expression(_parser);
        if (valueN == NULL) {
            __THROW_ERROR(
                _parser->fpath,
                _parser->fdata,
                _parser->current->position,
                "variable value expected"
            );
        }
        valus[index1++] = valueN;
        valus = (ast_node_list_t) realloc(valus, (sizeof(ast_node_t*) * (index1 + 2)));
        valus[index1] = NULL;
    } else {
        valus[index1++] = NULL;
        valus = (ast_node_list_t) realloc(valus, (sizeof(ast_node_t*) * (index1 + 2)));
        valus[index1] = NULL;
    }

    while (CHECKV(COMMA)) {
        ACCEPTV(COMMA);
        nameN = parser_terminal(_parser);
        if (nameN == NULL) {
            __THROW_ERROR(
                _parser->fpath,
                _parser->fdata,
                _parser->current->position,
                "variable name expected"
            );
        }
        names[index0++] = nameN;
        names = (ast_node_list_t) realloc(names, (sizeof(ast_node_t*) * (index0 + 2)));
        names[index0] = NULL;

        if (CHECKV(EQUAL)) {
            ACCEPTV(EQUAL);
            valueN = parser_expression(_parser);
            if (valueN == NULL) {
                __THROW_ERROR(
                    _parser->fpath,
                    _parser->fdata,
                    _parser->current->position,
                    "variable value expected"
                );
            }
            valus[index1++] = valueN;
            valus = (ast_node_list_t) realloc(valus, (sizeof(ast_node_t*) * (index1 + 2)));
            valus[index1] = NULL;
        } else {
            valus[index1++] = NULL;
            valus = (ast_node_list_t) realloc(valus, (sizeof(ast_node_t*) * (index1 + 2)));
            valus[index1] = NULL;
        }
    }
    ended = _parser->current->position;
    ACCEPTV(SEMICOLON);

    if (!_is_const && !_is_local) {
        return ast_var_statement_node(position_merge(start, ended), names, valus);
    } else if (_is_const) {
        return ast_const_statement_node(position_merge(start, ended), names, valus);
    } else if (_is_local) {
        return ast_local_statement_node(position_merge(start, ended), names, valus);
    } else {
        __THROW_ERROR(
            _parser->fpath,
            _parser->fdata,
            _parser->current->position,
            "invalid variable declaration"
        );
    }
}

ast_node_t* parser_if_statement(parser_t* _parser) {
    position_t* start = _parser->current->position, *ended = start;
    ACCEPTV(KEY_IF);
    ACCEPTV(LPAREN);
    ast_node_t* expr = parser_expression(_parser);
    if (expr == NULL) {
        __THROW_ERROR(
            _parser->fpath,
            _parser->fdata,
            _parser->current->position,
            "if condition expected"
        );
    }
    ACCEPTV(RPAREN);
    ast_node_t* trueNode = parser_statement(_parser);
    if (trueNode == NULL) {
        __THROW_ERROR(
            _parser->fpath,
            _parser->fdata,
            _parser->current->position,
            "if statement expected"
        );
    }
    ast_node_t* falseNode = NULL;
    if (CHECKV(KEY_ELSE)) {
        ACCEPTV(KEY_ELSE);
        falseNode = parser_statement(_parser);
        if (falseNode == NULL) {
            __THROW_ERROR(
                _parser->fpath,
                _parser->fdata,
                _parser->current->position,
                "else statement expected"
            );
        }
        ended = ast_position(falseNode);
    }
    return ast_if_statement_node(position_merge(start, ended), expr, trueNode, falseNode);
}

ast_node_t* parser_while_statement(parser_t* _parser) {
    position_t* start = _parser->current->position, *ended = start;
    ACCEPTV(KEY_WHILE);
    ACCEPTV(LPAREN);
    ast_node_t* expr = parser_expression(_parser);
    if (expr == NULL) {
        __THROW_ERROR(
            _parser->fpath,
            _parser->fdata,
            _parser->current->position,
            "while condition expected"
        );
    }
    ACCEPTV(RPAREN);
    ast_node_t* body = parser_statement(_parser);
    if (body == NULL) {
        __THROW_ERROR(
            _parser->fpath,
            _parser->fdata,
            _parser->current->position,
            "while body expected"
        );
    }
    ended = ast_position(body);
    return ast_while_statement_node(position_merge(start, ended), expr, body);
}

ast_node_t* parser_do_while_statement(parser_t* _parser) {
    position_t* start = _parser->current->position, *ended = start;
    ACCEPTV(KEY_DO);
    ast_node_t* body = parser_statement(_parser);
    if (body == NULL) {
        __THROW_ERROR(
            _parser->fpath,
            _parser->fdata,
            _parser->current->position,
            "do body expected"
        );
    }
    ACCEPTV(KEY_WHILE);
    ACCEPTV(LPAREN);
    ast_node_t* expr = parser_expression(_parser);
    if (expr == NULL) {
        __THROW_ERROR(
            _parser->fpath,
            _parser->fdata,
            _parser->current->position,
            "do while condition expected"
        );
    }
    ACCEPTV(RPAREN);
    ended = ast_position(body);
    return ast_do_while_statement_node(position_merge(start, ended), expr, body);
}

ast_node_t* parser_for_initializer(parser_t* _parser) {
    ast_node_t* k = parser_terminal(_parser);
    if (k == NULL) {
        __THROW_ERROR(
            _parser->fpath,
            _parser->fdata,
            _parser->current->position,
            "for initializer expected"
        );
    }
    if (!CHECKV(COMMA)) {
        return k;
    }
    ACCEPTV(COMMA);
    ast_node_t* v = parser_terminal(_parser);
    if (v == NULL) {
        __THROW_ERROR(
            _parser->fpath,
            _parser->fdata,
            _parser->current->position,
            "for variable expected"
        );
    }
    return ast_for_multiple_initializer_node(position_merge(ast_position(k), ast_position(v)), k, v);
}

ast_node_t* parser_for_statement(parser_t* _parser) {
    position_t* start = _parser->current->position, *ended = start;
    ACCEPTV(KEY_FOR);
    ACCEPTV(LPAREN);
    ast_node_t* initializer = parser_for_initializer(_parser);
    ACCEPTV(KEY_IN);
    ast_node_t* iterable = parser_expression(_parser);
    if (iterable == NULL) {
        __THROW_ERROR(
            _parser->fpath,
            _parser->fdata,
            _parser->current->position,
            "for iterable expected"
        );
    }
    ACCEPTV(RPAREN);
    ast_node_t* body = parser_statement(_parser);
    if (body == NULL) {
        __THROW_ERROR(
            _parser->fpath,
            _parser->fdata,
            _parser->current->position,
            "for body expected"
        );
    }
    ended = ast_position(body);
    ast_node_t* for_node = ast_for_statement_node(position_merge(start, ended), initializer, iterable, body);
    ast_node_list_t statements = (ast_node_list_t) malloc(sizeof(ast_node_t*) * 2);
    statements[0] = for_node;
    statements[1] = NULL;
    // Wrap into block statement to avoid scope issues
    return ast_block_statement_node(position_merge(start, ended), statements);
}

ast_node_t* parser_block_statement(parser_t* _parser) {
    position_t* start = _parser->current->position, *ended = start;
    ACCEPTV(LBRACKET);
    size_t index = 0;
    ast_node_list_t statements = (ast_node_list_t) malloc(sizeof(ast_node_t*));
    statements[0] = NULL;
    ast_node_t* statement = parser_statement(_parser);
    while (statement != NULL) {
        statements[index++] = statement;
        statements = (ast_node_list_t) realloc(statements, (sizeof(ast_node_t*) * (index + 1)));
        statements[index] = NULL;
        statement = parser_statement(_parser);
    }
    ended = _parser->current->position;
    ACCEPTV(RBRACKET);
    return ast_block_statement_node(position_merge(start, ended), statements);
}

ast_node_t* parser_return_statement(parser_t* _parser) {
    position_t* start = _parser->current->position, *ended = start;
    ACCEPTV(KEY_RETURN);
    ast_node_t* expr = parser_expression(_parser);
    ended = _parser->current->position;
    ACCEPTV(SEMICOLON);
    return ast_return_statement_node(position_merge(start, ended), expr);
}

ast_node_t* parser_program(parser_t* _parser) {
    size_t index = 0;
    ast_node_list_t list = (ast_node_list_t) malloc(sizeof(ast_node_t*));
    list[0] = NULL;

    position_t* start = _parser->current->position, *ended = start;

    ast_node_t* nodeN = parser_statement(_parser);
    if (nodeN != NULL) {
        ended = ast_position(nodeN);
    }
    while (nodeN != NULL) {
        list[index++] = nodeN;
        list = (ast_node_list_t) realloc(list, (sizeof(ast_node_t*) * (index + 1)));
        list[index] = NULL;
        nodeN = parser_statement(_parser);
        if (nodeN != NULL) {
            ended = ast_position(nodeN);
        }
    }
    ACCEPTT(TTEOF);
    return ast_program_node(
        position_merge(start, ended),
        list
    );
}

ast_node_t* parser_parse(parser_t* _parser) {
    _parser->current = tokenizer_next(_parser->tokenizer);
    return parser_program(_parser);
}

void parser_free(parser_t* _parser) {
    tokenizer_free(_parser->tokenizer);
    free(_parser);
}