#include "token.h"

token_t* token_new(token_type_t _type, char* _value, position_t* _position) {
    token_t* token = (token_t*) malloc(sizeof(token_t));
    token->type = _type;
    token->value = _value;
    token->position = _position;
    return token;
}

void token_free(token_t* _token) {
    free(_token->value);
    free(_token->position);
    free(_token);
}