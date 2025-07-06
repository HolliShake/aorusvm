#include "keyword.h"

const char* KEYWORDS[] = {
    KEY_VAR,
    KEY_CONST,
    KEY_LOCAL,
    KEY_FUNC,
    KEY_ASYNC,
    KEY_RETURN,
    KEY_IF,
    KEY_ELSE,
    KEY_FOR,
    KEY_WHILE,
    KEY_DO,
    KEY_BREAK,
    KEY_CONTINUE,
    KEY_TRUE,
    KEY_FALSE,
};

bool is_keyword(char* _value) {
    for (int i = 0; KEYWORDS[i] != NULL; i++) {
        if (strcmp(KEYWORDS[i], _value) == 0) {
            return true;
        }
    }
    return false;
}