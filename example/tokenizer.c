#include "tokenizer.h"

tokenizer_t* tokenizer_new(char* _path, char* _data) {
    tokenizer_t* tokenizer = (tokenizer_t*) malloc(sizeof(tokenizer_t));
    if (tokenizer == NULL) {
        PD("Failed to allocate memory for tokenizer");
    }
    tokenizer->fpath = _path;
    tokenizer->fdata = _data;
    tokenizer->look  = (strlen(_data) > 0) ? _data[0] : '\0';
    tokenizer->fsize = (strlen(_data));
    tokenizer->index = 0;
    tokenizer->line  = 1;
    tokenizer->colm  = 1;
    return tokenizer;
}

bool tokenizer_is_identifier_start(char _c) {
    return (_c >= 'a' && _c <= 'z') || (_c >= 'A' && _c <= 'Z') || _c == '_';
}

bool tokenizer_is_identifier_char(char _c) {
    return tokenizer_is_identifier_start(_c) || (_c >= '0' && _c <= '9');
}

bool tokenizer_is_whitespace(char _c) {
    return _c == ' ' || _c == '\t' || _c == '\n' || _c == '\r';
}

bool tokenizer_is_digit(char _c) {
    return _c >= '0' && _c <= '9';
}

bool tokenizer_is_str(char _c) {
    return _c == '"';
}

bool tokenizer_is_eof(tokenizer_t* _tokenizer) {
    return _tokenizer->index >= _tokenizer->fsize;
}

void tokenizer_forward(tokenizer_t* _tokenizer) {
    if (_tokenizer->look == '\n') {
        _tokenizer->line++;
        _tokenizer->colm = 1;
    } else {
        _tokenizer->colm++;
    }
    _tokenizer->index++;
    _tokenizer->look = (_tokenizer->index < _tokenizer->fsize) ? _tokenizer->fdata[_tokenizer->index] : '\0';
}

void tokenizer_skip_whitespace(tokenizer_t* _tokenizer) {
    while (tokenizer_is_whitespace(_tokenizer->look)) {
        tokenizer_forward(_tokenizer);
    }
}

#define init_str(outvar, _value) \
    char* outvar = (char*) malloc(strlen(_value) + 1); \
    strcpy(outvar, _value);

#define append_str(outvar, c) \
    { \
        size_t len = strlen(outvar); \
        outvar = (char*) realloc(outvar, len + 2); \
        outvar[len] = c; \
        outvar[len + 1] = '\0'; \
    } \


token_t* tokenizer_identifier(tokenizer_t* _tokenizer) {
    init_str(value, "");
    position_t* pos = position_from_line_and_colm(_tokenizer->line, _tokenizer->colm);
    while (!tokenizer_is_eof(_tokenizer) && tokenizer_is_identifier_char(_tokenizer->look)) {
        append_str(value, _tokenizer->look);
        tokenizer_forward(_tokenizer);
    }
    return token_new(is_keyword(value) ? TTKEY : TTIDN, value, pos);
}

token_t* tokenizer_number(tokenizer_t* _tokenizer) {
    init_str(value, "");
    position_t* pos = position_from_line_and_colm(_tokenizer->line, _tokenizer->colm);
    while (!tokenizer_is_eof(_tokenizer) && tokenizer_is_digit(_tokenizer->look)) {
        append_str(value, _tokenizer->look);
        tokenizer_forward(_tokenizer);
    }
    token_type_t type = TTINT;
    if (_tokenizer->look == '.') {
        type = TTNUM;
        append_str(value, _tokenizer->look);
        tokenizer_forward(_tokenizer);
        while (!tokenizer_is_eof(_tokenizer) && tokenizer_is_digit(_tokenizer->look)) {
            append_str(value, _tokenizer->look);
            tokenizer_forward(_tokenizer);
        }
    }
    return token_new(type, value, pos);
}

token_t* tokenizer_string(tokenizer_t* _tokenizer) {
    init_str(value, "");
    position_t* pos = position_from_line_and_colm(_tokenizer->line, _tokenizer->colm);
    bool is_open = tokenizer_is_str(_tokenizer->look), is_closed = false;
    tokenizer_forward(_tokenizer);
    is_closed = tokenizer_is_str(_tokenizer->look);
    while (!tokenizer_is_eof(_tokenizer) && !(is_open && is_closed)) {
        if (_tokenizer->look == '\n')
            break;
        if (_tokenizer->look == '\\') {
            tokenizer_forward(_tokenizer);
            switch (_tokenizer->look) {
                case 'b':
                    append_str(value, '\b');
                    break;
                case 'n':
                    append_str(value, '\f');
                    break;
                case 't':
                    append_str(value, '\t');
                    break;
                case 'r':
                    append_str(value, '\r');
                    break;
                case '\\':
                    append_str(value, '\\');
                    break;
                case '"':
                    append_str(value, '"');
                    break;
                default:
                    append_str(value, _tokenizer->look);
                    break;
            }
        } else {
            append_str(value, _tokenizer->look);
        }
        tokenizer_forward(_tokenizer);
        is_closed = tokenizer_is_str(_tokenizer->look);
    }
    if (!(is_open && is_closed)) {
        __THROW_ERROR(_tokenizer->fpath, _tokenizer->fdata, pos, "unclosed string");
    }
    tokenizer_forward(_tokenizer);
    return token_new(TTSTR, value, pos);
}

token_t* tokenizer_symbol(tokenizer_t* _tokenizer) {
    init_str(value, "");
    position_t* pos = position_from_line_and_colm(_tokenizer->line, _tokenizer->colm);
    switch (_tokenizer->look) {
        case '(':
        case ')':
        case '{':
        case '}':
        case '[':
        case ']':
        case ',':
        case ':':
        case ';': {
            append_str(value, _tokenizer->look);
            tokenizer_forward(_tokenizer);
            break;
        }
        case '.': {
            append_str(value, _tokenizer->look);
            tokenizer_forward(_tokenizer);
            if (_tokenizer->look == '.') {
                append_str(value, _tokenizer->look);
                tokenizer_forward(_tokenizer);
                if (_tokenizer->look != '.') {
                    __THROW_ERROR(_tokenizer->fpath, _tokenizer->fdata, pos, "invalid symbol");
                }
                append_str(value, _tokenizer->look);
                tokenizer_forward(_tokenizer);
            }
            break;
        }
        case '!': {
            append_str(value, _tokenizer->look);
            tokenizer_forward(_tokenizer);
            if (_tokenizer->look == '=') {
                append_str(value, _tokenizer->look);
                tokenizer_forward(_tokenizer);
            }
            break;
        }
        case '=': {
            append_str(value, _tokenizer->look);
            tokenizer_forward(_tokenizer);
            if (_tokenizer->look == '=') {
                append_str(value, _tokenizer->look);
                tokenizer_forward(_tokenizer);
            }
            break;
        }
        case '*': {
            append_str(value, _tokenizer->look);
            tokenizer_forward(_tokenizer);
            if (_tokenizer->look == '=') {
                append_str(value, _tokenizer->look);
                tokenizer_forward(_tokenizer);
            }
            break;
        }
        case '%': {
            append_str(value, _tokenizer->look);
            tokenizer_forward(_tokenizer);
            if (_tokenizer->look == '=') {
                append_str(value, _tokenizer->look);
                tokenizer_forward(_tokenizer);
            }
            break;
        }
        case '/': {
            append_str(value, _tokenizer->look);
            tokenizer_forward(_tokenizer);
            if (_tokenizer->look == '=') {
                append_str(value, _tokenizer->look);
                tokenizer_forward(_tokenizer);
            }
            break;
        }
        case '+': {
            append_str(value, _tokenizer->look);
            tokenizer_forward(_tokenizer);
            if (_tokenizer->look == '+' || _tokenizer->look == '=') {
                append_str(value, _tokenizer->look);
                tokenizer_forward(_tokenizer);
            }
            break;
        }
        case '-': {
            append_str(value, _tokenizer->look);
            tokenizer_forward(_tokenizer);
            if (_tokenizer->look == '-' || _tokenizer->look == '=') {
                append_str(value, _tokenizer->look);
                tokenizer_forward(_tokenizer);
            }
            break;
        }
        case '<': {
            append_str(value, _tokenizer->look);
            tokenizer_forward(_tokenizer);
            if (_tokenizer->look == '<') {
                append_str(value, _tokenizer->look);
                tokenizer_forward(_tokenizer);
            }
            if (_tokenizer->look == '=') {
                append_str(value, _tokenizer->look);
                tokenizer_forward(_tokenizer);
            }
            break;
        }
        case '>': {
            append_str(value, _tokenizer->look);
            tokenizer_forward(_tokenizer);
            if (_tokenizer->look == '>') {
                append_str(value, _tokenizer->look);
                tokenizer_forward(_tokenizer);
            }
            if (_tokenizer->look == '=') {
                append_str(value, _tokenizer->look);
                tokenizer_forward(_tokenizer);
            }
            break;
        }
        case '&': {
            append_str(value, _tokenizer->look);
            tokenizer_forward(_tokenizer);
            if (_tokenizer->look == '&' || _tokenizer->look == '=') {
                append_str(value, _tokenizer->look);
                tokenizer_forward(_tokenizer);
            }
            break;
        }
        case '|': {
            append_str(value, _tokenizer->look);
            tokenizer_forward(_tokenizer);
            if (_tokenizer->look == '|' || _tokenizer->look == '=') {
                append_str(value, _tokenizer->look);
                tokenizer_forward(_tokenizer);
            }
            break;
        }
        case '^': {
            append_str(value, _tokenizer->look);
            tokenizer_forward(_tokenizer);
            if (_tokenizer->look == '=') {
                append_str(value, _tokenizer->look);
                tokenizer_forward(_tokenizer);
            }
            break;
        }
        default: {
            __THROW_ERROR(_tokenizer->fpath, _tokenizer->fdata, pos, "invalid symbol");
        }
    }
    return token_new(TTSYM, value, pos);
}

token_t* tokenizer_next(tokenizer_t* _tokenizer) {
    while (!tokenizer_is_eof(_tokenizer)) {
        char c = _tokenizer->fdata[_tokenizer->index];
        if (tokenizer_is_whitespace(c)) {
            tokenizer_skip_whitespace(_tokenizer);
        } else if (tokenizer_is_identifier_start(c)) {
            return tokenizer_identifier(_tokenizer);
        } else if (tokenizer_is_digit(c)) {
            return tokenizer_number(_tokenizer);
        } else if (tokenizer_is_str(c)) {
            return tokenizer_string(_tokenizer);
        } else {
            return tokenizer_symbol(_tokenizer);
        }
    }
    return token_new(TTEOF, "EOF", position_from_line_and_colm(_tokenizer->line, _tokenizer->colm));
}

void tokenizer_free(tokenizer_t* _tokenizer) {
    free(_tokenizer);
}