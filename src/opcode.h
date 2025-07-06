#ifndef OPCODE_H
#define OPCODE_H

typedef enum opcode_enum {
    OPCODE_LOAD_NAME             = 75,   // Followed by the length of the name in bytes + 1 (for the null terminator)
    OPCODE_LOAD_INT              = 76,   // Followed by 4 bytes
    OPCODE_LOAD_DOUBLE           = 77,   // Followed by 8 bytes
    OPCODE_LOAD_BOOL             = 78,   // Followed by 1 byte
    OPCODE_LOAD_STRING           = 79,   // Followed by the length of the string in bytes + 1 (for the null terminator)
    OPCODE_LOAD_NULL             = 80,   // No following bytes
    OPCODE_STORE_NAME            = 81,   // Followed by the length of the name in bytes + 1 (for the null terminator)
    OPCODE_SET_NAME              = 82,   // Followed by the length of the name in bytes + 1 (for the null terminator)
    OPCODE_CALL                  = 83,   // Followed by 4 bytes (aka the number of arguments)
    OPCODE_INCREMENT             = 84,   // No following bytes
    OPCODE_MUL                   = 85,   // No following bytes
    OPCODE_DIV                   = 86,   // No following bytes
    OPCODE_MOD                   = 87,   // No following bytes
    OPCODE_ADD                   = 88,   // No following bytes
    OPCODE_SUB                   = 89,   // No following bytes
    OPCODE_SHL                   = 90,   // No following bytes
    OPCODE_SHR                   = 91,   // No following bytes
    OPCODE_CMP_LT                = 92,   // No following bytes
    OPCODE_CMP_LTE               = 93,   // No following bytes
    OPCODE_CMP_GT                = 94,   // No following bytes
    OPCODE_CMP_GTE               = 95,   // No following bytes
    OPCODE_CMP_EQ                = 96,   // No following bytes
    OPCODE_CMP_NE                = 97,   // No following bytes
    OPCODE_AND                   = 98,   // No following bytes
    OPCODE_OR                    = 99,   // No following bytes
    OPCODE_XOR                   = 100,  // No following bytes
    OPCODE_POP_JUMP_IF_FALSE     = 101,  // Followed by 4 bytes (aka jump offset)
    OPCODE_POP_JUMP_IF_TRUE      = 102,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_IF_FALSE_OR_POP  = 103,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_IF_TRUE_OR_POP   = 104,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_FORWARD          = 105,  // Followed by 4 bytes (aka jump offset)
    OPCODE_ABSOLUTE_JUMP         = 106,  // Followed by 4 bytes (aka jump offset)
    OPCODE_POPTOP                = 107,  // No following bytes
    OPCODE_NOP                   = 108,  // No following bytes
    OPCODE_MAKE_FUNCTION         = 109,  // Followed by 4 bytes (aka parameter count) + 8 bytes (aka bytecode size)
    OPCODE_MAKE_ASYNC_FUNCTION   = 110,  // Followed by 4 bytes (aka parameter count) + 8 bytes (aka bytecode size)
    OPCODE_SETUP_BLOCK           = 111,  // Followed by 8 bytes (aka bytecode size)
    OPCODE_RETURN                = 112,  // No following bytes
    OPCODE_COMPLETE_BLOCK        = 113,  // No following bytes
    OPCODE_DUPTOP                = 114   // No following bytes
} opcode_t;

#endif