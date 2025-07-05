#include "../src/api/ast/node.h"
#include "../src/api/ast/position.h"
#include "keyword.h"
#include "token.h"
#include "tokenizer.h"

#ifndef PARSER_H
#define PARSER_H


typedef struct parser_struct {
    char*        fpath;
    char*        fdata;
    tokenizer_t* tokenizer;
    token_t*     current;
} parser_t;

parser_t* parser_new(char* _path, char* _data);

ast_node_t* parser_parse(parser_t* _parser);

void parser_free(parser_t* _parser);

#endif