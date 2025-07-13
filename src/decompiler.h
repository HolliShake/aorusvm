#include "api/core/global.h"
#include "api/core/internal.h"
#include "opcode.h"

#ifndef DECOMPILER_H
#define DECOMPILER_H

void decompile(uint8_t* _bytecode, bool _with_header);

#endif