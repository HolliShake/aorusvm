#include "../src/api/core/global.h"

#ifndef KEYWORD_H
#define KEYWORD_H

#define KEY_VAR "var"
#define KEY_CONST "const"
#define KEY_LOCAL "local"
#define KEY_CLASS "class"
#define KEY_EXTENDS "extends"
#define KEY_FUNC "func"
#define KEY_ASYNC "async"
#define KEY_RETURN "return"
#define KEY_IF "if"
#define KEY_ELSE "else"
#define KEY_SWITCH "switch"
#define KEY_FOR "for"
#define KEY_WHILE "while"
#define KEY_DO "do"
#define KEY_FOR "for"
#define KEY_IN "in"
#define KEY_BREAK "break"
#define KEY_CONTINUE "continue"
#define KEY_TRUE "true"
#define KEY_FALSE "false"
#define KEY_NULL "null"
#define KEY_THIS "this"
#define KEY_SUPER "super"
#define KEY_NEW "new"
#define KEY_AWAIT "await"
#define KEY_CATCH "catch"

extern const char* KEYWORDS[];

bool is_keyword(char* _value);

#endif