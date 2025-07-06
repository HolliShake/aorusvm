#ifndef OPCODE_H
#define OPCODE_H

typedef enum opcode_enum {
    OPCODE_LOAD_NAME             = 75,   // Followed by the length of the name in bytes + 1 (for the null terminator)
    OPCODE_LOAD_INT              = 76,   // Followed by 4 bytes
    OPCODE_LOAD_DOUBLE           = 77,   // Followed by 8 bytes
    OPCODE_LOAD_BOOL             = 78,   // Followed by 1 byte
    OPCODE_LOAD_STRING           = 79,   // Followed by the length of the string in bytes + 1 (for the null terminator)
    OPCODE_LOAD_NULL             = 80,   // No following bytes
    OPCODE_LOAD_ARRAY            = 81,   // Followed by 4 bytes (aka the number of elements)
    OPCODE_EXTEND_ARRAY          = 82,   // No following bytes
    OPCODE_APPEND_ARRAY          = 83,   // No following bytes
    OPCODE_STORE_NAME            = 84,   // Followed by the length of the name in bytes + 1 (for the null terminator)
    OPCODE_SET_NAME              = 85,   // Followed by the length of the name in bytes + 1 (for the null terminator)
    OPCODE_CALL                  = 86,   // Followed by 4 bytes (aka the number of arguments)
    OPCODE_INCREMENT             = 87,   // No following bytes
    OPCODE_MUL                   = 88,   // No following bytes
    OPCODE_DIV                   = 89,   // No following bytes
    OPCODE_MOD                   = 90,   // No following bytes
    OPCODE_ADD                   = 91,   // No following bytes
    OPCODE_SUB                   = 92,   // No following bytes
    OPCODE_SHL                   = 93,   // No following bytes
    OPCODE_SHR                   = 94,   // No following bytes
    OPCODE_CMP_LT                = 95,   // No following bytes
    OPCODE_CMP_LTE               = 96,   // No following bytes
    OPCODE_CMP_GT                = 97,   // No following bytes
    OPCODE_CMP_GTE               = 98,   // No following bytes
    OPCODE_CMP_EQ                = 99,   // No following bytes
    OPCODE_CMP_NE                = 100,  // No following bytes
    OPCODE_AND                   = 101,  // No following bytes
    OPCODE_OR                    = 102,  // No following bytes
    OPCODE_XOR                   = 103,  // No following bytes
    OPCODE_POP_JUMP_IF_FALSE     = 104,  // Followed by 4 bytes (aka jump offset)
    OPCODE_POP_JUMP_IF_TRUE      = 105,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_IF_FALSE_OR_POP  = 106,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_IF_TRUE_OR_POP   = 107,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_FORWARD          = 108,  // Followed by 4 bytes (aka jump offset)
    OPCODE_ABSOLUTE_JUMP         = 109,  // Followed by 4 bytes (aka jump offset)
    OPCODE_POPTOP                = 110,  // No following bytes
    OPCODE_NOP                   = 111,  // No following bytes
    OPCODE_MAKE_FUNCTION         = 112,  // Followed by 4 bytes (aka parameter count) + 8 bytes (aka bytecode size)
    OPCODE_MAKE_ASYNC_FUNCTION   = 113,  // Followed by 4 bytes (aka parameter count) + 8 bytes (aka bytecode size)
    OPCODE_SETUP_BLOCK           = 114,  // Followed by 8 bytes (aka bytecode size)
    OPCODE_RETURN                = 115,  // No following bytes
    OPCODE_COMPLETE_BLOCK        = 116,  // No following bytes
    OPCODE_DUPTOP                = 117,  // No following bytes
    OPCODE_ROT2                  = 118,  // No following bytes
} opcode_t;

#endif