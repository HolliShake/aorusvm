#include "../src/api/ast/position.h"
#include "../src/api/core/error.h"
#include "../src/api/core/global.h"
#include "keyword.h"
#include "token.h"

#ifndef TOKENIZER_H
#define TOKENIZER_H

typedef struct tokenizer_struct {
    char*  fpath;
    char*  fdata;
    char   look;
    size_t fsize;
    size_t index;
    size_t line;
    size_t colm;
} tokenizer_t;

tokenizer_t* tokenizer_new(char* _path, char* _data);

bool tokenizer_is_eof(tokenizer_t* _tokenizer);

token_t* tokenizer_next(tokenizer_t* _tokenizer);

void tokenizer_free(tokenizer_t* _tokenizer);

#endif