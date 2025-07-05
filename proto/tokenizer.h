#include "../src/api/core/global.h"
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

#endif