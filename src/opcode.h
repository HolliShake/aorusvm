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
    OPCODE_LOAD_OBJECT           = 84,   // Followed by 4 bytes (aka the number of properties)
    X0                           = 85,
    X1                           = 86,
    OPCODE_STORE_NAME            = 87,   // Followed by the length of the name in bytes + 1 (for the null terminator)
    OPCODE_SET_NAME              = 88,   // Followed by the length of the name in bytes + 1 (for the null terminator)
    OPCODE_CALL                  = 89,   // Followed by 4 bytes (aka the number of arguments)
    OPCODE_INCREMENT             = 90,   // No following bytes
    OPCODE_MUL                   = 91,   // No following bytes
    OPCODE_DIV                   = 92,   // No following bytes
    OPCODE_MOD                   = 93,   // No following bytes
    OPCODE_ADD                   = 94,   // No following bytes
    OPCODE_SUB                   = 95,   // No following bytes
    OPCODE_SHL                   = 96,   // No following bytes
    OPCODE_SHR                   = 97,   // No following bytes
    OPCODE_CMP_LT                = 98,   // No following bytes
    OPCODE_CMP_LTE               = 99,   // No following bytes
    OPCODE_CMP_GT                = 100,  // No following bytes
    OPCODE_CMP_GTE               = 101,  // No following bytes
    OPCODE_CMP_EQ                = 102,  // No following bytes
    OPCODE_CMP_NE                = 103,  // No following bytes
    OPCODE_AND                   = 104,  // No following bytes
    OPCODE_OR                    = 105,  // No following bytes
    OPCODE_XOR                   = 106,  // No following bytes
    OPCODE_POP_JUMP_IF_FALSE     = 107,  // Followed by 4 bytes (aka jump offset)
    OPCODE_POP_JUMP_IF_TRUE      = 108,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_IF_FALSE_OR_POP  = 109,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_IF_TRUE_OR_POP   = 110,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_FORWARD          = 111,  // Followed by 4 bytes (aka jump offset)
    OPCODE_ABSOLUTE_JUMP         = 112,  // Followed by 4 bytes (aka jump offset)
    OPCODE_POPTOP                = 113,  // No following bytes
    OPCODE_NOP                   = 114,  // No following bytes
    OPCODE_MAKE_FUNCTION         = 115,  // Followed by 4 bytes (aka parameter count) + 8 bytes (aka bytecode size)
    OPCODE_MAKE_ASYNC_FUNCTION   = 116,  // Followed by 4 bytes (aka parameter count) + 8 bytes (aka bytecode size)
    OPCODE_SETUP_BLOCK           = 117,  // Followed by 8 bytes (aka bytecode size)
    OPCODE_RETURN                = 118,  // No following bytes
    OPCODE_COMPLETE_BLOCK        = 119,  // No following bytes
    OPCODE_DUPTOP                = 120,  // No following bytes
    OPCODE_ROT2                  = 121,  // No following bytes
} opcode_t;

#endif