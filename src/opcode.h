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
    OPCODE_STORE_NAME            = 82,   // Followed by the length of the name in bytes + 1 (for the null terminator)
    OPCODE_SET_NAME              = 83,   // Followed by the length of the name in bytes + 1 (for the null terminator)
    OPCODE_CALL                  = 84,   // Followed by 4 bytes (aka the number of arguments)
    OPCODE_INCREMENT             = 85,   // No following bytes
    OPCODE_MUL                   = 86,   // No following bytes
    OPCODE_DIV                   = 87,   // No following bytes
    OPCODE_MOD                   = 88,   // No following bytes
    OPCODE_ADD                   = 89,   // No following bytes
    OPCODE_SUB                   = 90,   // No following bytes
    OPCODE_SHL                   = 91,   // No following bytes
    OPCODE_SHR                   = 92,   // No following bytes
    OPCODE_CMP_LT                = 93,   // No following bytes
    OPCODE_CMP_LTE               = 94,   // No following bytes
    OPCODE_CMP_GT                = 95,   // No following bytes
    OPCODE_CMP_GTE               = 96,   // No following bytes
    OPCODE_CMP_EQ                = 97,   // No following bytes
    OPCODE_CMP_NE                = 98,   // No following bytes
    OPCODE_AND                   = 99,   // No following bytes
    OPCODE_OR                    = 100,  // No following bytes
    OPCODE_XOR                   = 101,  // No following bytes
    OPCODE_POP_JUMP_IF_FALSE     = 102,  // Followed by 4 bytes (aka jump offset)
    OPCODE_POP_JUMP_IF_TRUE      = 103,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_IF_FALSE_OR_POP  = 104,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_IF_TRUE_OR_POP   = 105,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_FORWARD          = 106,  // Followed by 4 bytes (aka jump offset)
    OPCODE_ABSOLUTE_JUMP         = 107,  // Followed by 4 bytes (aka jump offset)
    OPCODE_POPTOP                = 108,  // No following bytes
    OPCODE_NOP                   = 109,  // No following bytes
    OPCODE_MAKE_FUNCTION         = 110,  // Followed by 4 bytes (aka parameter count) + 8 bytes (aka bytecode size)
    OPCODE_MAKE_ASYNC_FUNCTION   = 111,  // Followed by 4 bytes (aka parameter count) + 8 bytes (aka bytecode size)
    OPCODE_SETUP_BLOCK           = 112,  // Followed by 8 bytes (aka bytecode size)
    OPCODE_RETURN                = 113,  // No following bytes
    OPCODE_COMPLETE_BLOCK        = 114,  // No following bytes
    OPCODE_DUPTOP                = 115   // No following bytes
} opcode_t;

#endif