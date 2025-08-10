#include "api/core/global.h"
#include "api/core/internal.h"
#include "code.h"
#include "opcode.h"

#ifndef DECOMPILER_H
#define DECOMPILER_H

void decompile(code_t* _bytecode, bool _with_header);
void decompile_with_ip(code_t* _code, size_t _ip);

#endif