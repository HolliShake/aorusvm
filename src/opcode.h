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
    OPCODE_MUL                   = 82,   // No following bytes
    OPCODE_DIV                   = 83,   // No following bytes
    OPCODE_MOD                   = 84,   // No following bytes
    OPCODE_ADD                   = 85,   // No following bytes
    OPCODE_SUB                   = 86,   // No following bytes
    OPCODE_SHL                   = 87,   // No following bytes
    OPCODE_SHR                   = 88,   // No following bytes
    OPCODE_CMP_LT                = 89,   // No following bytes
    OPCODE_CMP_LTE               = 90,   // No following bytes
    OPCODE_CMP_GT                = 91,   // No following bytes
    OPCODE_CMP_GTE               = 92,   // No following bytes
    OPCODE_CMP_EQ                = 93,   // No following bytes
    OPCODE_CMP_NE                = 94,   // No following bytes
    OPCODE_AND                   = 95,   // No following bytes
    OPCODE_OR                    = 96,   // No following bytes
    OPCODE_XOR                   = 97,   // No following bytes
    OPCODE_POP_JUMP_IF_FALSE     = 98,   // Followed by 4 bytes (aka jump offset)
    OPCODE_POP_JUMP_IF_TRUE      = 99,   // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_IF_FALSE_OR_POP  = 100,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_IF_TRUE_OR_POP   = 101,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_FORWARD          = 102,  // Followed by 4 bytes (aka jump offset)
    OPCODE_POPTOP                = 103,  // No following bytes
    OPCODE_NOP                   = 104,  // No following bytes
    OPCODE_RETURN                = 105   // No following bytes
} opcode_t;

#endif