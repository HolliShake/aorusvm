#include "../src/api/ast/position.h"
#include "../src/api/core/global.h"

#ifndef TOKEN_H
#define TOKEN_H

typedef enum token_type_enum {
    TTKEY,
    TTIDN,
    TTINT,
    TTNUM,
    TTSTR,
    TTSYM,
    TTEOF
} token_type_t;

typedef struct token_struct {
    token_type_t type;
    char* value;
    position_t* position;
} token_t;

token_t* token_new(token_type_t _type, char* _value, position_t* _position);

void token_free(token_t* _token);

#endif