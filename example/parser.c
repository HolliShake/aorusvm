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
        "unexpected token: %s", _parser->current->value
    );
}

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
    } else {
        return NULL;
    }
}

ast_node_t* parser_multiplicative(parser_t* _parser) {
    ast_node_t* node = parser_terminal(_parser);
    if (node == NULL) {
        return NULL;
    }
    while (CHECKV("*") || CHECKV("/") || CHECKV("%")) {
        char* op = _parser->current->value;
        ACCEPTV(op);
        ast_node_t* right = parser_terminal(_parser);
        if (right == NULL) {
            __THROW_ERROR(
                _parser->fpath,
                _parser->fdata,
                _parser->current->position,
                "missing right operand for %s", op
            );
        }
        if (strcmp(op, "*") == 0) {
            node = ast_binary_mul_node(
                position_merge(ast_position(node), ast_position(right)), 
                node, 
                right
            );
        } else if (strcmp(op, "/") == 0) {
            node = ast_binary_div_node(
                position_merge(ast_position(node), ast_position(right)), 
                node, 
                right
            );
        } else if (strcmp(op, "%") == 0) {
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
    while (CHECKV("+") || CHECKV("-")) {
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
        if (strcmp(op, "+") == 0) {
            node = ast_binary_add_node(
                position_merge(ast_position(node), ast_position(right)), 
                node, 
                right
            );
        } else if (strcmp(op, "-") == 0) {
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
    while (CHECKV("<<") || CHECKV(">>")) {
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
        if (strcmp(op, "<<") == 0) {
            node = ast_binary_shl_node(
                position_merge(ast_position(node), ast_position(right)), 
                node, 
                right
            );
        } else if (strcmp(op, ">>") == 0) {
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
    while (CHECKV("<") || CHECKV("<=") || CHECKV(">") || CHECKV(">=")) {
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
        if (strcmp(op, "<") == 0) {
            node = ast_cmp_lt_node(
                position_merge(ast_position(node), ast_position(right)), 
                node, 
                right
            );
        } else if (strcmp(op, "<=") == 0) {
            node = ast_cmp_lte_node(
                position_merge(ast_position(node), ast_position(right)), 
                node, 
                right
            );
        } else if (strcmp(op, ">") == 0) {
            node = ast_cmp_gt_node(
                position_merge(ast_position(node), ast_position(right)), 
                node, 
                right
            );
        } else if (strcmp(op, ">=") == 0) {
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
    while (CHECKV("==") || CHECKV("!=")) {
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
        if (strcmp(op, "==") == 0) {
            node = ast_cmp_eq_node(
                position_merge(ast_position(node), ast_position(right)), 
                node, 
                right
            );
        } else if (strcmp(op, "!=") == 0) {
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
    while (CHECKV("&") || CHECKV("|") || CHECKV("^")) {
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
        if (strcmp(op, "&") == 0) {
            node = ast_binary_and_node(
                position_merge(ast_position(node), ast_position(right)), 
                node, 
                right
            );
        } else if (strcmp(op, "|") == 0) {
            node = ast_binary_or_node(
                position_merge(ast_position(node), ast_position(right)), 
                node, 
                right
            );
        } else if (strcmp(op, "^") == 0) {
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
    while (CHECKV("&&") || CHECKV("||")) {
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

ast_node_t* parser_expression(parser_t* _parser) {
    return parser_logical(_parser);
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

ast_node_t* parser_variable_declaration(parser_t* _parser);
ast_node_t* parser_if_statement(parser_t* _parser);

ast_node_t* parser_statement(parser_t* _parser) {
    position_t* start = _parser->current->position, *ended = start;
    if (CHECKV(KEY_VAR)) {
        return parser_variable_declaration(_parser);
    } else if (CHECKV(KEY_IF)) {
        return parser_if_statement(_parser);
    }
    /************************************/
    ast_node_t* node = parser_logical(_parser);
    if (node == NULL) {
        return NULL;
    }
    ended = _parser->current->position;
    ACCEPTV(";");
    ast_node_t* expr = ast_statement_expression_node(
        position_merge(start, ended),
        node
    );
    return expr;
}

ast_node_t* parser_variable_declaration(parser_t* _parser) {
    position_t* start = _parser->current->position, *ended = start;
    size_t index0 = 0, index1 = 0;
    ast_node_list_t names = (ast_node_list_t) malloc(sizeof(ast_node_t*));
    ast_node_list_t valus = (ast_node_list_t) malloc(sizeof(ast_node_t*));
    names[0] = NULL;
    valus[0] = NULL;

    ACCEPTV(KEY_VAR);
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

    if (CHECKV("=")) {
        ACCEPTV("=");
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

    while (CHECKV(",")) {
        ACCEPTV(",");
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

        if (CHECKV("=")) {
            ACCEPTV("=");
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
    ACCEPTV(";");

    return ast_var_statement_node(position_merge(start, ended), names, valus);
}

ast_node_t* parser_if_statement(parser_t* _parser) {
    position_t* start = _parser->current->position, *ended = start;
    ACCEPTV(KEY_IF);
    ACCEPTV("(");
    ast_node_t* expr = parser_expression(_parser);
    if (expr == NULL) {
        __THROW_ERROR(
            _parser->fpath,
            _parser->fdata,
            _parser->current->position,
            "if condition expected"
        );
    }
    ACCEPTV(")");
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
        list = (ast_node_list_t) realloc(list, (sizeof(ast_node_t*) * (index + 2)));
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