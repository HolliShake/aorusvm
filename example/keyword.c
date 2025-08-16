#include "keyword.h"

const char* KEYWORDS[] = {
    KEY_VAR,
    KEY_CONST,
    KEY_LOCAL,
    KEY_CLASS,
    KEY_EXTENDS,
    KEY_FUNC,
    KEY_ASYNC,
    KEY_RETURN,
    KEY_IF,
    KEY_ELSE,
    KEY_SWITCH,
    KEY_FOR,
    KEY_WHILE,
    KEY_DO,
    KEY_FOR,
    KEY_IN,
    KEY_BREAK,
    KEY_CONTINUE,
    KEY_TRUE,
    KEY_FALSE,
    KEY_NULL,
    KEY_NEW,
    KEY_THIS,
    KEY_SUPER,
    KEY_AWAIT,
    KEY_CATCH,
    NULL
};

bool is_keyword(char* _value) {
    for (int i = 0; KEYWORDS[i] != NULL; i++) {
        if (strcmp(KEYWORDS[i], _value) == 0) {
            return true;
        }
    }
    return false;
}